/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#include <cstring>
#include <stdexcept>
#include "emulator.hpp"
#include "../cart/sram.hpp"
#include "../cart/flash.hpp"
#include "util/file.hpp"
#include "util/logger.hpp"
#include "util/likely.hpp"

using namespace Util;

namespace GameBoyAdvance {

    constexpr int Emulator::cycles[16];
    constexpr int Emulator::cycles32[16];

    Emulator::Emulator(Config* config) : config(config), ppu(config), apu(config) {

        reset();

        // setup interrupt controller
        m_interrupt.set_flag_register(&regs.irq.if_);

        // feed PPU with important data.
        ppu.set_memory(memory.palette, memory.oam, memory.vram);
        ppu.set_interrupt(&m_interrupt);
    }

    Emulator::~Emulator() {

    }

    void Emulator::reset() {
        auto& ctx = get_context();

        ARM::reset();

        // reset PPU und APU state
        ppu.reset();
        apu.reset();

        // reset cartridge memory
        if (memory.rom.save != nullptr) {
            memory.rom.save->reset();
        }

        // clear out all memory
        memset(memory.wram, 0, 0x40000);
        memset(memory.iram, 0, 0x8000);
        memset(memory.palette,  0, 0x400);
        memset(memory.oam,  0, 0x400);
        memset(memory.vram, 0, 0x18000);
        memset(memory.mmio, 0, 0x800);

        // reset IO-registers
        regs.irq.ie  = 0;
        regs.irq.if_ = 0;
        regs.irq.ime = 0;
        regs.haltcnt = SYSTEM_RUN;
        regs.keyinput = 0x3FF;

        for (int i = 0; i < 4; i++) {
            // reset DMA channels
            regs.dma[i].id = i;
            regs.dma[i].reset();

            // !!hacked!! ouchy ouch
            regs.dma[i].dma_active  = &dma_running;
            regs.dma[i].current_dma = &dma_current;

            // reset timers
            regs.timer[i].id = i;
            regs.timer[i].reset();
        }

        cycles_left = 0;
        dma_running  = false;
        dma_current = 0;

        set_fake_swi(!config->use_bios);

        if (!config->use_bios || !File::exists(config->bios_path)) {
            // TODO: load helper BIOS

            // set CPU entrypoint to ROM entrypoint
            ctx.r15     = 0x08000000;
            ctx.reg[13] = 0x03007F00;

            // set stack pointers to their respective addresses
            ctx.bank[BANK_SVC][BANK_R13] = 0x03007FE0;
            ctx.bank[BANK_IRQ][BANK_R13] = 0x03007FA0;

            // load first two ROM instructions
            refill_pipeline();
        } else {
            int size = File::get_size(config->bios_path);
            u8* data = File::read_data(config->bios_path);

            if (size > 0x4000) {
                throw std::runtime_error("bad BIOS image");
            }

            // copy BIOS to local buffer
            memcpy(memory.bios, data, size);

            // reset r15 because of weird reason
            ctx.r15 = 0x00000000;

            delete data;
        }

        memory.bios_opcode = 0;
    }

    APU& Emulator::get_apu() {
        return apu;
    }

    u16& Emulator::get_keypad() {
        return regs.keyinput;
    }

    void Emulator::load_config() {
        ppu.load_config();
        apu.load_config();
    }

    void Emulator::load_game(std::shared_ptr<Cartridge> cart) {
        // hold reference to the Cartridge
        this->cart = cart;

        // internal copies, for optimization.
        this->memory.rom.data = cart->data;
        this->memory.rom.size = cart->size;
        this->memory.rom.save = cart->backup;

        reset();
    }

    void Emulator::frame() {
        const int VISIBLE_LINES   = 160;
        const int INVISIBLE_LINES = 68;

        const int CYCLES_ACTIVE = 960;
        const int CYCLES_HBLANK = 272;
        const int CYCLES_ENTIRE = CYCLES_ACTIVE + CYCLES_HBLANK;

        int frame_count = config->fast_forward ? config->multiplier : 1;

        for (int frame = 0; frame < frame_count; frame++) {
            // 160 visible lines, alternating SCANLINE and HBLANK.
            for (int line = 0; line < VISIBLE_LINES; line++) {
                // SCANLINE
                ppu.scanline(frame == 0);
                run_for(CYCLES_ACTIVE);

                // HBLANK
                ppu.hblank();
                if (!dma_running) {
                    dma_hblank();
                }
                run_for(CYCLES_HBLANK);

                ppu.next_line();
                apu.step(CYCLES_ENTIRE);
            }

            // 68 invisible lines, VBLANK period.
            ppu.vblank();
            if (!dma_running) {
                dma_vblank();
            }
            for (int line = 0; line < INVISIBLE_LINES; line++) {
                run_for(CYCLES_ENTIRE);
                ppu.next_line();
                apu.step(CYCLES_ENTIRE);
            }
        }
    }

    void Emulator::run_for(int cycles) {
        int cycles_previous;

        cycles_left += cycles;

        while (cycles_left > 0) {
            u32 requested_and_enabled = regs.irq.if_ & regs.irq.ie;

            if (regs.haltcnt == SYSTEM_HALT && requested_and_enabled) {
                regs.haltcnt = SYSTEM_RUN;
            }

            cycles_previous = cycles_left;

            if (UNLIKELY(dma_running)) {
                dma_transfer();
            } else if (LIKELY(regs.haltcnt == SYSTEM_RUN)) {
                if (regs.irq.ime && requested_and_enabled) {
                    signal_interrupt();
                }
                step();
            } else {
                //TODO: inaccurate because of timer interrupts
                timer_step(cycles_left);
                cycles_left = 0;
                return;
            }

            timer_step(cycles_previous - cycles_left);
        }

        //if (cycles_left < -10) {
        //    fmt::print("desync {0} cycles!\n", cycles_left*-1);
        //}
    }
}

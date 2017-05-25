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

    Emulator::Emulator(Config* config) : m_config(config), m_ppu(config), m_apu(config) {

        reset();

        // setup interrupt controller
        m_interrupt.set_flag_register(&m_io.interrupt.request);

        // feed PPU with important data.
        m_ppu.set_memory(memory.palette, memory.oam, memory.vram);
        m_ppu.set_interrupt(&m_interrupt);
    }

    Emulator::~Emulator() {

    }

    void Emulator::reset() {
        auto& ctx = get_context();

        ARM::reset();

        // reset PPU und APU state
        m_ppu.reset();
        m_apu.reset();

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
        memset(m_mmio, 0, 0x800);

        // reset IO-registers
        m_io.keyinput = 0x3FF;
        m_io.interrupt.reset();
        m_io.haltcnt = SYSTEM_RUN;

        for (int i = 0; i < 4; i++) {
            // reset DMA channels
            m_io.dma[i].id = i;
            m_io.dma[i].reset();

            // !!hacked!! ouchy ouch
            m_io.dma[i].dma_active  = &m_dma_active;
            m_io.dma[i].current_dma = &m_current_dma;

            // reset timers
            m_io.timer[i].id = i;
            m_io.timer[i].reset();
        }

        m_cycles = 0;
        m_dma_active  = false;
        m_current_dma = 0;

        set_fake_swi(!m_config->use_bios);

        if (!m_config->use_bios || !File::exists(m_config->bios_path)) {
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
            int size = File::get_size(m_config->bios_path);
            u8* data = File::read_data(m_config->bios_path);

            if (size > 0x4000) {
                throw std::runtime_error("bad BIOS image");
            }

            // copy BIOS to local buffer
            memcpy(memory.bios, data, size);

            // reset r15 because of weird reason
            ctx.r15 = 0x00000000;

            delete data;
        }

        bios_opcode = 0;
    }

    APU& Emulator::get_apu() {
        return m_apu;
    }

    u16& Emulator::get_keypad() {
        return m_io.keyinput;
    }

    void Emulator::load_config() {
        m_ppu.load_config();
        m_apu.load_config();
    }

    void Emulator::load_game(std::shared_ptr<Cartridge> cart) {
        // hold reference to the Cartridge
        this->cart = cart;

        // internal copies, for optimization.
        this->memory.rom.data      = cart->data;
        this->memory.rom.size = cart->size;
        this->memory.rom.save   = cart->backup;

        reset();
    }

    void Emulator::frame() {
        const int VISIBLE_LINES   = 160;
        const int INVISIBLE_LINES = 68;

        const int CYCLES_ACTIVE = 960;
        const int CYCLES_HBLANK = 272;
        const int CYCLES_ENTIRE = CYCLES_ACTIVE + CYCLES_HBLANK;

        int frame_count = m_config->fast_forward ? m_config->multiplier : 1;

        for (int frame = 0; frame < frame_count; frame++) {
            // 160 visible lines, alternating SCANLINE and HBLANK.
            for (int line = 0; line < VISIBLE_LINES; line++) {
                // SCANLINE
                m_ppu.scanline(frame == 0);
                run_for(CYCLES_ACTIVE);

                // HBLANK
                m_ppu.hblank();
                if (!m_dma_active) {
                    dma_hblank();
                }
                run_for(CYCLES_HBLANK);

                m_ppu.next_line();
                m_apu.step(CYCLES_ENTIRE);
            }

            // 68 invisible lines, VBLANK period.
            m_ppu.vblank();
            if (!m_dma_active) {
                dma_vblank();
            }
            for (int line = 0; line < INVISIBLE_LINES; line++) {
                run_for(CYCLES_ENTIRE);
                m_ppu.next_line();
                m_apu.step(CYCLES_ENTIRE);
            }
        }
    }

    void Emulator::run_for(int cycles) {
        int cycles_previous;

        m_cycles += cycles;

        while (m_cycles > 0) {
            u32 requested_and_enabled = m_io.interrupt.request & m_io.interrupt.enable;

            if (m_io.haltcnt == SYSTEM_HALT && requested_and_enabled) {
                m_io.haltcnt = SYSTEM_RUN;
            }

            cycles_previous = m_cycles;

            if (UNLIKELY(m_dma_active)) {
                dma_transfer();
            } else if (LIKELY(m_io.haltcnt == SYSTEM_RUN)) {
                if (m_io.interrupt.master_enable && requested_and_enabled) {
                    signal_interrupt();
                }
                step();
            } else {
                //TODO: inaccurate because of timer interrupts
                timer_step(m_cycles);
                m_cycles = 0;
                return;
            }

            timer_step(cycles_previous - m_cycles);
        }

        //if (m_cycles < -10) {
        //    fmt::print("desync {0} cycles!\n", m_cycles*-1);
        //}
    }
}

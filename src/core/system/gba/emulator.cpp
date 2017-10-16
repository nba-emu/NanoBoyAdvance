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

#include "util/file.hpp"
#include "util/logger.hpp"
#include "util/likely.hpp"

using namespace Util;

namespace Core {

    constexpr int Emulator::s_ws_nseq[4];
    constexpr int Emulator::s_ws_seq0[2];
    constexpr int Emulator::s_ws_seq1[2];
    constexpr int Emulator::s_ws_seq2[2];

    constexpr u32 Emulator::s_dma_dst_mask[4];
    constexpr u32 Emulator::s_dma_src_mask[4];
    constexpr u32 Emulator::s_dma_len_mask[4];

    Emulator::Emulator(Config* config) : config(config), ppu(config, memory.palette, memory.oam, memory.vram), apu(config) {
        //// must be initialized *before* calling reset()
        //memory.rom.save = nullptr;
        //Emulator::reset();

        // setup interrupt controller
        m_interrupt.set_flag_register(&regs.irq.flag);

        // feed PPU with important data. (EEK!)
        ppu.setInterruptController(&m_interrupt);
    }

    Emulator::~Emulator() {
    }

    void Emulator::reset() {
        auto& ctx = context();

        ARM::reset();

        // reset PPU und APU state
        ppu.reset();
        apu.reset();

        // reset cartridge memory
        if (memory.rom.save != nullptr) {
            memory.rom.save->reset();
        }

        // clear out all memory
        memset(memory.wram,    0, 0x40000);
        memset(memory.iram,    0, 0x08000);
        memset(memory.palette, 0, 0x00400);
        memset(memory.oam,     0, 0x00400);
        memset(memory.vram,    0, 0x18000);
        memset(memory.mmio,    0, 0x00800);

        // reset IO-registers
        regs.irq.enable        = 0;
        regs.irq.flag          = 0;
        regs.irq.master_enable = 0;
        regs.haltcnt           = SYSTEM_RUN;
        regs.keyinput          = 0x3FF;

        // reset WAITCNT
        auto& waitcnt = regs.waitcnt;
        {
            waitcnt.sram     = 0;
            waitcnt.ws0_n    = 0;
            waitcnt.ws0_s    = 0;
            waitcnt.ws1_n    = 0;
            waitcnt.ws1_s    = 0;
            waitcnt.ws2_n    = 0;
            waitcnt.ws2_s    = 0;
            waitcnt.phi      = 0;
            waitcnt.prefetch = 0;
            waitcnt.cgb      = 0;

            // setup 8/16/32 bit access cycle LUT
            for (int i = 0; i < 2; i++) {
                cycles[i][0x0] = 1; cycles32[i][0x0] = 1;
                cycles[i][0x1] = 1; cycles32[i][0x1] = 1;
                cycles[i][0x2] = 3; cycles32[i][0x2] = 6; // BEWARE! - see 4000800h
                cycles[i][0x3] = 1; cycles32[i][0x3] = 1;
                cycles[i][0x4] = 1; cycles32[i][0x4] = 1;
                cycles[i][0x5] = 1; cycles32[i][0x5] = 2;
                cycles[i][0x6] = 1; cycles32[i][0x6] = 2;
                cycles[i][0x7] = 1; cycles32[i][0x7] = 1;
                cycles[i][0xF] = 1; cycles32[i][0xF] = 1;
            }
            calculateMemoryCycles();
        }

        // Reset DMA & Timer states.
        for (int i = 0; i < 4; i++) {
            dmaReset(i);
            timerReset(i);
        }

        cycles_left = 0;
        dma_running = 0;
        dma_current = 0;

        swiHLE(!config->use_bios);

        if (!config->use_bios || !File::exists(config->bios_path)) {
            // TODO: load helper BIOS

            // set CPU entrypoint to ROM entrypoint
            ctx.r15     = 0x08000000;
            ctx.reg[13] = 0x03007F00;

            // set stack pointers to their respective addresses
            ctx.bank[BANK_SVC][BANK_R13] = 0x03007FE0;
            ctx.bank[BANK_IRQ][BANK_R13] = 0x03007FA0;

            // load first two ROM instructions
            refillPipeline();
        } else {
            int size = File::get_size(config->bios_path);
            u8* data = File::read_data(config->bios_path);

            if (size > 0x4000) {
                throw std::runtime_error("bad BIOS image");
            }

            // copy BIOS to local buffer
            memcpy(memory.bios, data, size);

            // start at BIOS reset vector
            ctx.r15 = 0x00000000;

            delete data;
        }

        memory.bios_opcode = 0;
    }

    APU& Emulator::getAPU() {
        return apu;
    }

    u16& Emulator::getKeypad() {
        return regs.keyinput;
    }

    void Emulator::setKeyState(Key key, bool pressed) {
        // NOTE: GBA keys work with pull up logic
        if (pressed) {
            regs.keyinput &= ~static_cast<int>(key);
        } else {
            regs.keyinput |=  static_cast<int>(key);
        }
    }

    void Emulator::reloadConfig() {
        ppu.reloadConfig();
        apu.reloadConfig();
    }

    void Emulator::loadGame(std::shared_ptr<Cartridge> cart) {
        // hold reference to the Cartridge
        this->cart = cart;

        // internal copies, for optimization.
        this->memory.rom.data = cart->data;
        this->memory.rom.size = cart->size;
        this->memory.rom.save = cart->backup;

        reset();
    }

    void Emulator::runFrame() {
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
                runInternal(CYCLES_ACTIVE);

                // HBLANK
                ppu.hblank();
                dmaFindHBlank();
                runInternal(CYCLES_HBLANK);

                ppu.nextLine();
                apu.step(CYCLES_ENTIRE);
            }

            // 68 invisible lines, VBLANK period.
            ppu.vblank();
            dmaFindVBlank();
            for (int line = 0; line < INVISIBLE_LINES; line++) {
                runInternal(CYCLES_ENTIRE);
                ppu.nextLine();
                apu.step(CYCLES_ENTIRE);
            }
        }
    }

    void Emulator::runInternal(int cycles) {
        int cycles_previous;

        cycles_left += cycles;

        while (cycles_left > 0) {
            u32 requested_and_enabled = regs.irq.flag & regs.irq.enable;

            if (regs.haltcnt == SYSTEM_HALT && requested_and_enabled) {
                regs.haltcnt = SYSTEM_RUN;
            }

            cycles_previous = cycles_left;

            if (UNLIKELY(dma_running != 0)) {
                dmaTransfer();
            } else if (LIKELY(regs.haltcnt == SYSTEM_RUN)) {
                if (regs.irq.master_enable && requested_and_enabled) {
                    signalIRQ();
                }
                step();
            } else {
                //TODO: inaccurate because of timer interrupts
                timerStep(cycles_left);
                cycles_left = 0;
                return;
            }

            timerStep(cycles_previous - cycles_left);
        }
    }

    // TODO: consider placing this in memory.hpp?
    void Emulator::calculateMemoryCycles() {
        const auto& waitcnt = regs.waitcnt;

        // SRAM cycles. I assume it's the same for every type of access?
        cycles[0][0xE]   = s_ws_nseq[waitcnt.sram];
        cycles[1][0xE]   = s_ws_nseq[waitcnt.sram];
        cycles32[0][0xE] = s_ws_nseq[waitcnt.sram];
        cycles32[1][0xE] = s_ws_nseq[waitcnt.sram];

        // WS0/WS1/WS2 non-sequential cycles
        cycles[0][0x8] = cycles[0][0x9] = 1 + s_ws_nseq[waitcnt.ws0_n];
        cycles[0][0xA] = cycles[0][0xB] = 1 + s_ws_nseq[waitcnt.ws1_n];
        cycles[0][0xC] = cycles[0][0xD] = 1 + s_ws_nseq[waitcnt.ws2_n];

        // WS0/WS1/WS2 sequential cycles
        cycles[1][0x8] = cycles[1][0x9] = 1 + s_ws_seq0[waitcnt.ws0_s];
        cycles[1][0xA] = cycles[1][0xB] = 1 + s_ws_seq1[waitcnt.ws1_s];
        cycles[1][0xC] = cycles[1][0xD] = 1 + s_ws_seq2[waitcnt.ws2_s];

        // WS0/WS1/WS2 32-bit non-sequential access: 1N access, 1S access
        cycles32[0][0x8] = cycles32[0][0x9] = cycles[0][0x8] + cycles[1][0x8];
        cycles32[0][0xA] = cycles32[0][0xB] = cycles[0][0xA] + cycles[1][0x8];
        cycles32[0][0xC] = cycles32[0][0xD] = cycles[0][0xC] + cycles[1][0x8];

        // WS0/WS1/WS2 32-bit sequential access: 2S accesses
        cycles32[1][0x8] = cycles32[1][0x9] = cycles[1][0x8] * 2;
        cycles32[1][0xA] = cycles32[1][0xB] = cycles[1][0xA] * 2;
        cycles32[1][0xC] = cycles32[1][0xD] = cycles[1][0xC] * 2;
    }
}

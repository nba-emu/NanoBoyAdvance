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
#include "cpu.hpp"
#include "flash.hpp"
#include "util/logger.hpp"

using namespace util;

namespace GameBoyAdvance {
    constexpr int CPU::m_mem_cycles8_16[16];
    constexpr int CPU::m_mem_cycles32[16];
    constexpr CPU::read_func CPU::m_read_table[16];
    constexpr CPU::write_func CPU::m_write_table[16];

    CPU::CPU() {
        reset();

        // setup interrupt controller
        m_interrupt.set_flag_register(&m_io.interrupt.request);

        // feed PPU with important data.
        m_ppu.set_memory(m_pal, m_oam, m_vram);
        m_ppu.set_interrupt(&m_interrupt);
    }

    CPU::~CPU() {
        if (m_backup != nullptr) {
            delete m_backup;
            m_backup = nullptr;
        }
    }

    void CPU::reset() {
        ARM::reset();

        m_ppu.reset();

        if (m_backup != nullptr) {
            m_backup->reset();
        }

        // clear out all memory
        memset(m_wram, 0, 0x40000);
        memset(m_iram, 0, 0x8000);
        memset(m_pal,  0, 0x400);
        memset(m_oam,  0, 0x400);
        memset(m_vram, 0, 0x18000);

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

        set_hle(false); // temporary

        if (m_hle) {
            m_reg[15] = 0x08000000;
            m_reg[13] = 0x03007F00;
            m_bank[BANK_SVC][BANK_R13] = 0x03007FE0;
            m_bank[BANK_IRQ][BANK_R13] = 0x03007FA0;
            refill_pipeline();
        }
    }

    PPU& CPU::get_ppu() {
        return m_ppu;
    }

    u16& CPU::get_keypad() {
        return m_io.keyinput;
    }

    void CPU::set_bios(u8* data, size_t size) {
        if (size <= sizeof(m_bios)) {
            memcpy(m_bios, data, size);
            return;
        }
        throw std::runtime_error("bios file is too big.");
    }

    void CPU::set_game(u8* data, size_t size, std::string save_file) {
        m_rom      = data;
        m_rom_size = size;

        if (m_backup != nullptr) {
            delete m_backup;
            m_backup = nullptr;
        }

        // detect save type...
        for (int i = 0; i < size; i += 4) {
            if (memcmp(data + i, "EEPROM_V", 8) == 0) {
                // ...
            } else if (memcmp(data + i, "SRAM_V", 6) == 0) {
                // ...
            } else if (memcmp(data + i, "FLASH_V", 7) == 0 ||
                     memcmp(data + i, "FLASH512_V", 10) == 0) {
                m_backup = new Flash(save_file, false);
            } else if (memcmp(data + i, "FLASH1M_V", 9) == 0) {
                m_backup = new Flash(save_file, true);
            }
        }
    }

    void CPU::frame() {
        const int VISIBLE_LINES   = 160;
        const int INVISIBLE_LINES = 68;
        
        const int CYCLES_ACTIVE = 960;
        const int CYCLES_HBLANK = 272;
        const int CYCLES_ENTIRE = CYCLES_ACTIVE + CYCLES_HBLANK;
        
        // 160 visible lines, alternating SCANLINE and HBLANK.
        for (int line = 0; line < VISIBLE_LINES; line++) {
            // SCANLINE
            m_ppu.scanline();
            run_for(CYCLES_ACTIVE);

            // HBLANK
            m_ppu.hblank();
            if (!m_dma_active) {
                dma_hblank();
            }
            run_for(CYCLES_HBLANK);

            m_ppu.next_line();
        }

        // 68 invisible lines, VBLANK period.
        m_ppu.vblank();
        if (!m_dma_active) {
            dma_vblank();
        }
        for (int line = 0; line < INVISIBLE_LINES; line++) {
            run_for(CYCLES_ENTIRE);
            m_ppu.next_line();
        }
    }

    void CPU::run_for(int cycles) {
        int cycles_previous;

        m_cycles += cycles;

        while (m_cycles >= 0) {
            u32 requested_and_enabled = m_io.interrupt.request & m_io.interrupt.enable;

            if (m_io.haltcnt == SYSTEM_HALT && requested_and_enabled) {
                m_io.haltcnt = SYSTEM_RUN;
            }

            if (m_io.haltcnt == SYSTEM_RUN) {
                cycles_previous = m_cycles;

                if (!m_dma_active) {
                    if (m_io.interrupt.master_enable && requested_and_enabled) {
                        raise_interrupt();
                    }
                    step();
                } else {
                    dma_transfer();
                }

                timer_step(cycles_previous - m_cycles);
            } else {
                if (m_dma_active) {
                    cycles_previous = m_cycles;
                    
                    dma_transfer();
                    timer_step(cycles_previous - m_cycles);
                } else {
                    // TODO(accuracy): run timers only until first IRQ
                    timer_step(m_cycles);
                    
                    m_cycles = 0;
                    return;
                }
            }
        }

        //if (m_cycles < -10) {
        //    fmt::print("desync {0} cycles!\n", m_cycles*-1);
        //}
    }
}

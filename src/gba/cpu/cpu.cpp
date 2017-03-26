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
#include "../cart/flash.hpp"
#include "util/file.hpp"
#include "util/logger.hpp"
#include "util/likely.hpp"

using namespace Util;

namespace GameBoyAdvance {
    
    constexpr int CPU::m_mem_cycles8_16[16];
    constexpr int CPU::m_mem_cycles32[16];
    constexpr CPU::read_func  CPU::m_read_table[16];
    constexpr CPU::write_func CPU::m_write_table[16];

    CPU::CPU(Config* config) : m_config(config), m_ppu(config), m_apu(config) {
        
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
        }
        if (m_rom != nullptr) {
            delete m_rom;
        }
    }

    void CPU::reset() {
        ARM::reset();

        // reset PPU und APU state
        m_ppu.reset();
        m_apu.reset();

        // reset cartridge memory
        if (m_backup != nullptr) {
            m_backup->reset();
        }

        // clear out all memory
        memset(m_wram, 0, 0x40000);
        memset(m_iram, 0, 0x8000);
        memset(m_pal,  0, 0x400);
        memset(m_oam,  0, 0x400);
        memset(m_vram, 0, 0x18000);
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

        set_hle(!m_config->use_bios);

        if (!m_config->use_bios || !File::exists(m_config->bios_path)) {
            // TODO: load helper BIOS
            
            // set CPU entrypoint to ROM entrypoint
            m_reg[15] = 0x08000000;
            m_reg[13] = 0x03007F00;
            
            // set stack pointers to their respective addresses
            m_bank[BANK_SVC][BANK_R13] = 0x03007FE0;
            m_bank[BANK_IRQ][BANK_R13] = 0x03007FA0;
            
            // load first two ROM instructions
            refill_pipeline();
        } else {
            int size = File::get_size(m_config->bios_path);
            u8* data = File::read_data(m_config->bios_path);
            
            if (size > 0x4000) {
                throw std::runtime_error("bad BIOS image");
            }
            
            // copy BIOS to local buffer
            memcpy(m_bios, data, size);
            
            // reset r15 because of weird reason
            m_reg[15] = 0x00000000;
            
            delete data;
        }
    }

    APU& CPU::get_apu() {
        return m_apu;
    }
    
    u16& CPU::get_keypad() {
        return m_io.keyinput;
    }

    void CPU::load_config() {
        m_ppu.load_config();
        m_apu.load_config();
    }
    
    void CPU::load_game(std::string rom_file, std::string save_file) {
        m_rom      = File::read_data(rom_file);
        m_rom_size = File::get_size(rom_file);

        if (m_backup != nullptr) {
            delete m_backup;
            m_backup = nullptr;
        }

        SaveType save_type = m_config->save_type;
        
        if (save_type == SAVE_DETECT) {
            // Quick way for determining the save type.
            // Might want to add an overwrite for nasty games that like to trick emulators
            for (int i = 0; i < m_rom_size; i += 4) {
                if (memcmp(m_rom + i, "EEPROM_V", 8) == 0) {
                    save_type = SAVE_EEPROM;
                } else if (memcmp(m_rom + i, "SRAM_V", 6) == 0) {
                    save_type = SAVE_SRAM;
                } else if (memcmp(m_rom + i, "FLASH_V"   , 7 ) == 0 ||
                           memcmp(m_rom + i, "FLASH512_V", 10) == 0) {
                    save_type = SAVE_FLASH64;
                } else if (memcmp(m_rom + i, "FLASH1M_V", 9) == 0) {
                    save_type = SAVE_FLASH128;
                }
            }
        }
        
        switch (save_type) {
            case SAVE_EEPROM:   break;
            case SAVE_SRAM:     break;
            case SAVE_FLASH64:  m_backup = new Flash(save_file, false); break;
            case SAVE_FLASH128: m_backup = new Flash(save_file, true); break;
        }
        
        // forced system reset...
        reset();
    }

    void CPU::frame() {
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

    void CPU::run_for(int cycles) {
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
                    raise_interrupt();
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

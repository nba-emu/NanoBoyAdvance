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

#include "flash.hpp"
#include "util/file.hpp"
#include "util/logger.hpp"

using namespace Util;

namespace Core
{
    Flash::Flash(std::string save_file, bool second_bank) {
        m_save_file   = save_file;
        m_second_bank = second_bank;

        reset();
    }

    Flash::~Flash() {
        int size = m_second_bank ? 131072 : 65536;

        File::write_data(m_save_file, (u8*)m_memory, size);
    }

    void Flash::reset() {
        u8* save_data;

        // reset internal state
        m_bank           = 0;
        m_command_phase  = 0;
        m_enable_chip_id = false;
        m_enable_erase   = false;
        m_enable_write   = false;
        m_enable_bank_select = false;

        if (File::exists(m_save_file)) {
            int size = File::get_size(m_save_file);

            // validate save size
            if (size == (m_second_bank ? 131072 : 65536)) {
                save_data = File::read_data(m_save_file);
                for (int i = 0; i < 65536; i++) {
                    m_memory[0][i] = save_data[i];
                    m_memory[1][i] = m_second_bank ? save_data[65536 + i] : 0;
                }
                return;
            } else {
                Logger::log<LOG_WARN>("insane save size: {0}. file contents will be truncated.", size);
            }
        } else {
            // no save file was found - clear content
            for (int i = 0; i < 65536; i++) {
                m_memory[0][i] = 0xFF;
                m_memory[1][i] = 0xFF;
            }
        }
    }

    auto Flash::read8(u32 address) -> u8 {
        address &= 0xFFFF;

        // TODO(accuracy): according to VisualBoyAdvance chip id might be mirrored each 100h bytes.
        if (m_enable_chip_id && address < 2) {
            // Chip identifier for FLASH64: D4BF (SST 64K)
            // Chip identifier for FLASH128: 09C2 (Macronix 128K)
            if (m_second_bank) {
                return (address == 0) ? 0xC2 : 0x09;
            } else {
                return (address == 0) ? 0xBF : 0xD4;
            }
        }

        return m_memory[m_bank][address];
    }

    void Flash::write8(u32 address, u8 value) {
        if (m_enable_write) {
            // single (actual) byte write
            m_memory[m_bank][address & 0xFFFF] = value;
            m_enable_write = false;
            return;
        }

        if (address == 0x0E005555 && value == 0xAA) {
            // first stage of command submission:
            // command transmission is initiated through setting 0x0E005555 = 0xAA.
            m_command_phase = 1;
        } else if (address == 0x0E002AAA && value == 0x55 && m_command_phase == 1) {
            // second stage of command submission:
            // command transmission furtherly initiated by setting 0x0E002AAA = 0x55.
            m_command_phase = 2;
        } else if (address == 0x0E005555 && m_command_phase == 2) {
            // third stage of command submission:
            // actual command id is written to port 0x0E005555.
            switch (static_cast<FlashCommand>(value)) {
                case READ_CHIP_ID:   m_enable_chip_id = true;  return;
                case FINISH_CHIP_ID: m_enable_chip_id = false; return;
                case ERASE:          m_enable_erase   = true;  return;
                case ERASE_CHIP:
                    if (m_enable_erase) {
                        for (int i = 0; i < 65536; i++) {
                            m_memory[0][i] = 0xFF;
                            m_memory[1][i] = 0xFF;
                        }
                        m_enable_erase = false;
                    }
                    return;
                case WRITE_BYTE: m_enable_write = true; return;
                case SELECT_BANK:
                    if (m_second_bank) {
                        m_enable_bank_select = true;
                    }
                    return;
                    
                default: return;
            }

            // 0E005555=AA, 0E002AAA=55 sequence must be performed again before
            // another command id may be transferred to the chip.
            m_command_phase = 0;
        } else if (m_enable_erase && static_cast<FlashCommand>(value) == ERASE_SECTOR && (
                 address & ~0xF000) == 0x0E000000 && m_command_phase == 2) {
            
            // erase single sector
            int base_address = address & 0xF000;

            for (int i = 0; i < 0x1000; i++) {
                m_memory[m_bank][base_address + i] = 0xFF;
            }
                
            m_enable_erase  = false;
            m_command_phase = 0;
        } else if (m_enable_bank_select && address == 0x0E000000) {
            // bank Selection
            m_bank = value & 1;
            m_enable_bank_select = false;
        }
    }
}

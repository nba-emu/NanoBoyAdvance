///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2017 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "cart_backup.hpp"

namespace gba {
    class Flash : public CartBackup {
    private:
        enum FlashCommand {
            READ_CHIP_ID   = 0x90,
            FINISH_CHIP_ID = 0xF0,
            ERASE          = 0x80,
            ERASE_CHIP     = 0x10,
            ERASE_SECTOR   = 0x30,
            WRITE_BYTE     = 0xA0,
            SELECT_BANK    = 0xB0
        };

        int  m_bank;
        bool m_second_bank;
        u8   m_memory[2][65536];
        int  m_command_phase;
        bool m_enable_chip_id;
        bool m_enable_erase;
        bool m_enable_write;
        bool m_enable_bank_select;

        std::string m_save_file;

    public:
        Flash(std::string save_file, bool second_bank);
        ~Flash();

        void reset();
        auto read_byte(u32 address) -> u8;
        void write_byte(u32 address, u8 value);
    };
}

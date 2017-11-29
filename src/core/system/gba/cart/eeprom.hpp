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

#pragma once

#include <string>
#include <vector>
#include "save.hpp"

namespace Core {

    class EEPROM : public Save {
    public:
        enum EEPROMSize {
            SIZE_4K  = 0,
            SIZE_64K = 1
        };

        EEPROM(std::string save_path, EEPROMSize size_hint);
       ~EEPROM();

        void reset();
        auto read8(u32 address) -> u8;
        void write8(u32 address, u8 value);

    private:
        void resetSerialBuffer();

        enum State {
            STATE_ACCEPT_COMMAND = 1 << 0,
            STATE_READ_MODE      = 1 << 1,
            STATE_WRITE_MODE     = 1 << 2,
            STATE_GET_ADDRESS    = 1 << 3,
            STATE_READING        = 1 << 4,
            STATE_DUMMY_NIBBLE   = 1 << 5,
            STATE_WRITING        = 1 << 6,
            STATE_EAT_DUMMY      = 1 << 7
        };

        static constexpr int s_addr_bits[2] = { 6, 14 };
        static constexpr int s_save_size[2] = { 512, 8192 };

        std::string save_path;

        u8* memory {nullptr};
        int memory_size;

        int address;
        u64 serial_buffer;
        int transmitted_bits;

        int state;
        EEPROMSize size;
    };
}

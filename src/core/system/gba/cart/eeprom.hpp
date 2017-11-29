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
    enum EEPROMSize {
        EEPROM_4K  = 0,
        EEPROM_64K = 1
    };

    class EEPROM : public Save {
    public:
        EEPROM(std::string save_file, EEPROMSize size);
       ~EEPROM();

        void reset();
        auto read8(u32 address) -> u8;
        void write8(u32 address, u8 value);

    private:
        void resetSerialBuffer();

        enum State {
            EEPROM_ACCEPT_COMMAND = 1 << 0,
            EEPROM_READ_MODE      = 1 << 1,
            EEPROM_WRITE_MODE     = 1 << 2,
            EEPROM_GET_ADDRESS    = 1 << 3,
            EEPROM_READING        = 1 << 4,
            EEPROM_DUMMY_NIBBLE   = 1 << 5,
            EEPROM_WRITING        = 1 << 6,
            EEPROM_EAT_DUMMY      = 1 << 7
        };

        static constexpr int s_addr_bits[2] = { 6, 14 };

        u8* memory {nullptr};

        u64 serialBuffer;
        int transmittedBits;
        int currentAddress;

        int state;
        EEPROMSize size;

        int bufferSize;
    };
}

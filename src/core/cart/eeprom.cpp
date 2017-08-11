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

#include "eeprom.hpp"

#include <iostream>

namespace GameBoyAdvance {
    EEPROM::EEPROM(std::string save_path) {
    }

    EEPROM::~EEPROM() {
    }

    void EEPROM::reset() {
    }

    auto EEPROM::read8(u32 address) -> u8 {
        std::cout << "read! " << std::hex << address << std::dec << std::endl;
        return 0xFF;
    }

    void EEPROM::write8(u32 address, u8 value) {
        value &= 1;

        std::cout << std::hex << address << ": " << (int)value << std::dec << std::endl;
    }
}

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
#include "save.hpp"

#define SRAM_SIZE 65536

namespace Core {
    class SRAM : public Save {
    private:
        
        u8 m_memory[SRAM_SIZE];
        std::string m_save_file;

    public:
        SRAM(std::string save_file);
        ~SRAM();

        void reset();
        auto read8(u32 address) -> u8;
        void write8(u32 address, u8 value);
    };
}

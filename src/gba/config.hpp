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
#include "util/integer.hpp"

namespace GameBoyAdvance {
    
    enum SaveType {
        SAVE_DETECT,
        SAVE_SRAM,
        SAVE_FLASH64,
        SAVE_FLASH128,
        SAVE_EEPROM
    };
    
    struct Config {
        // Core
        bool use_bios = true;
        std::string bios_path;
        SaveType save_type = SAVE_DETECT;
        
        // Graphics
        int  frameskip     = 0;
        u32* framebuffer   = nullptr;
        bool darken_screen = false;
    };
}
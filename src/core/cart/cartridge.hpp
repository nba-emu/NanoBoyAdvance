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
#include <memory>
#include "header.hpp"
#include "save.hpp"

namespace Core {
    
    enum SaveType {
        SAVE_DETECT,
        SAVE_SRAM,
        SAVE_FLASH64,
        SAVE_FLASH128,
        SAVE_EEPROM
    };
    
    struct Cartridge {
        u32 size;
        union {
            u8*     data;
            Header* header;
        };
        SaveType type;
        Save* backup;

        Cartridge() : data(nullptr), backup(nullptr) { }
        
        ~Cartridge() {
            delete data;
            delete backup;
        }
        
        auto detectType() -> SaveType;
        
        static auto fromFile(std::string path, SaveType type = SAVE_DETECT) -> std::shared_ptr<Cartridge>;
    };
    
}
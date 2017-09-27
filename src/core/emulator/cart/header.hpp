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

#include "util/integer.hpp"

namespace Core {
    
    #pragma pack(push, 1) 
    
    // For details see: http://problemkaputt.de/gbatek.htm#gbacartridgeheader
    struct Header {
        // First instruction
        u32 entrypoint;
        
        // Compressed Bitmap (Nintendo Logo)
        u8 nintendo_logo[156];
        
        // Game Title, Code and Maker Code
        struct {
            char title[12];
            char code[4];
            char maker[2];
        } game;
        
        u8 fixed_96h;
        u8 unit_code;
        u8 device_type;
        u8 reserved[7];
        u8 version;
        u8 checksum;
        u8 reserved2[2];
        
        // Multiboot Header
        struct {
            u32 ram_entrypoint;
            u8  boot_mode;
            u8  slave_id;
            u8  unused[26];
            u32 joy_entrypoint;
        } mb;
    };
    
    #pragma pack(pop)
}
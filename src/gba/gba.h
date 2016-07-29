/*
* Copyright (C) 2016 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "arm.h"
#include "memory.h"
#include "util/types.h"
#include "util/file.h"
#include <string>

namespace NanoboyAdvance
{
    class GBA
    {
        static const int FRAME_CYCLES;
        
        int speed_multiplier {1};
        bool did_render {false};

        ARM7* arm;
        GBAMemory* memory;
    public:
        enum class Key
        {
            None = 0,
            A = 1,
            B = 2,
            Select = 4,
            Start = 8,
            Right = 16,
            Left = 32,
            Up = 64,
            Down = 128,
            R = 256,
            L = 512
        };

        GBA(std::string rom_file, std::string save_file);
        GBA(std::string rom_file, std::string save_file, std::string bios_file);
        ~GBA();

        void Frame();
        void SetKeyState(Key key, bool pressed);
        void SetSpeedUp(int multiplier);
        u32* GetVideoBuffer();
        bool HasRendered();
    };
};
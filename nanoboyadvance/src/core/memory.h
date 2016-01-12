/*
* Copyright (C) 2015 Frederic Meyer
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

#include "common/types.h"

using namespace std;

namespace NanoboyAdvance
{ 
    class Memory
    { 
    public:
        virtual u8 ReadByte(u32 offset) { return 0; }
        virtual u16 ReadHWord(u32 offset) { return 0; }
        virtual u32 ReadWord(u32 offset) { return 0; }
        virtual void WriteByte(u32 offset, u8 value) {}
        virtual void WriteHWord(u32 offset, u16 value) {}
        virtual void WriteWord(u32 offset, u32 value) {}
    };
}

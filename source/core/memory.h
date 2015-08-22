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

#include "../common/types.h"

namespace NanoboyAdvance
{ 
    class Memory
    { 
    public:
        virtual ubyte ReadByte(uint offset) { return 0; }
        virtual ushort ReadHWord(uint offset) { return 0; }
        virtual uint ReadWord(uint offset) { return 0; }
        virtual void WriteByte(uint offset, ubyte value) {}
        virtual void WriteHWord(uint offset, ushort value) {}
        virtual void WriteWord(uint offset, uint value) {}
    };
}

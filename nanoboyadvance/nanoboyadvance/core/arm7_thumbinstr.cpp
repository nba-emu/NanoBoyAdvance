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

#include "arm7.h"

#define THUMB_1 1
#define THUMB_2 2
#define THUMB_3 3
#define THUMB_4 4
#define THUMB_5 5
#define THUMB_6 6
#define THUMB_7 7
#define THUMB_8 8
#define THUMB_9 9
#define THUMB_10 10
#define THUMB_11 11
#define THUMB_12 12
#define THUMB_13 13
#define THUMB_14 14
#define THUMB_15 15
#define THUMB_16 16
#define THUMB_17 17
#define THUMB_18 18
#define THUMB_19 19

namespace NanoboyAdvance
{
    int ARM7::THUMBDecode(ushort instruction)
    {
        if ((instruction & 0xF800) < 0x1800)
        {
            // THUMB.1 Move shifted register
            return THUMB_1;
        }
        else if ((instruction & 0xF800) == 0x1800)
        {
            // THUMB.2 Add/subtract
            return THUMB_2;
        }
        else if ((instruction & 0xE000) == 0x2000)
        {
            // THUMB.3 Move/compare/add/subtract immediate
            return THUMB_3;
        }
        else if ((instruction & 0xFC00) == 0x4000)
        {
            // THUMB.4 ALU operations
            return THUMB_4;
        }
        else if ((instruction & 0xFC00) == 0x4400)
        {
            // THUMB.5 Hi register operations/branch exchange
            return THUMB_5;
        }
        else if ((instruction & 0xF800) == 0x4800)
        {
            // THUMB.6 PC-relative load
            return THUMB_6;
        }
        else if ((instruction & 0xF200) == 0x5000)
        {
            // THUMB.7 Load/store with register offset
            return THUMB_7;
        }
        else if ((instruction & 0xF200) == 0x5200)
        {
            // THUMB.8 Load/store sign-extended byte/halfword
            return THUMB_8;
        }
        else if ((instruction & 0xE000) == 0x6000)
        {
            // THUMB.9 Load store with immediate offset
            return THUMB_9;
        }
        else if ((instruction & 0xF000) == 0x8000)
        {
            // THUMB.10 Load/store halfword
            return THUMB_10;
        }
        else if ((instruction & 0xF000) == 0x9000)
        {
            // THUMB.11 SP-relative load/store
            return THUMB_11;
        }
        else if ((instruction & 0xF000) == 0xA000)
        {
            // THUMB.12 Load address
            return THUMB_12;
        }
        else if ((instruction & 0xFF00) == 0xB000)
        {
            // THUMB.13 Add offset to stack pointer
            return THUMB_13;
        }
        else if ((instruction & 0xF600) == 0xB400)
        {
            // THUMB.14 push/pop registers
            return THUMB_14;
        }
        else if ((instruction & 0xF000) == 0xC000)
        {
            // THUMB.15 Multiple load/store
            return THUMB_15;
        }
        else if ((instruction & 0xF000) == 0xD000)
        {
            // THUMB.16 Conditional Branch
            return THUMB_16;
        }
        else if ((instruction & 0xFF00) == 0xDF00)
        {
            // THUMB.17 Software Interrupt
            return THUMB_17;
        }
        else if ((instruction & 0xF800) == 0xE000)
        {
            // THUMB.18 Unconditional Branch
            return THUMB_18;
        }
        else if ((instruction & 0xF000) == 0xF000)
        {
            // THUMB.19 Long branch with link
            return THUMB_19;
        }
        return 0;
    }
}
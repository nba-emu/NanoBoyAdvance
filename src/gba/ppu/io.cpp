///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2017 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#include "ppu.hpp"

namespace gba
{
    u8 ppu::io::status_reg::read(int offset)
    {
        switch (offset)
        {
        case 0:
            return (vblank_flag ? 1 : 0) |
                   (hblank_flag ? 2 : 0) |
                   (vcount_flag ? 4 : 0) |
                   (vblank_interrupt ? 8 : 0) |
                   (hblank_interrupt ? 16 : 0) |
                   (vcount_interrupt ? 32 : 0);
        case 1:
            return vcount_setting;
        }
    }

    void ppu::io::status_reg::write(int offset, u8 value)
    {
        switch (offset)
        {
        case 0:
            vblank_interrupt = value & 8;
            hblank_interrupt = value & 16;
            vcount_interrupt = value & 32;
            break;
        case 1:
            vcount_setting = value;
            break;
        }
    }
}

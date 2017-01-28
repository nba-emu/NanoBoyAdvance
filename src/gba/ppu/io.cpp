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
    void ppu::io::control_reg::reset()
    {
        mode = 0;
        cgb_mode = false;
        frame_select = 0;
        hblank_oam_access = false;
        one_dimensional = false;
        forced_blank = false;
        enable[0] = false;
        enable[1] = false;
        enable[2] = false;
        enable[3] = false;
        enable[4] = false;
        win_enable[0] = false;
        win_enable[1] = false;
        win_enable[2] = false;
    }

    u8 ppu::io::control_reg::read(int offset)
    {
        switch (offset)
        {
        case 0:
            return mode |
                   (cgb_mode ? 8 : 0) |
                   (frame_select ? 16 : 0) |
                   (hblank_oam_access ? 32 : 0) |
                   (one_dimensional ? 64 : 0) |
                   (forced_blank ? 128 : 0);
        case 1:
            return (enable[0] ? 1 : 0) |
                   (enable[1] ? 2 : 0) |
                   (enable[2] ? 4 : 0) |
                   (enable[3] ? 8 : 0) |
                   (enable[4] ? 16 : 0) |
                   (win_enable[0] ? 32 : 0) |
                   (win_enable[1] ? 64 : 0) |
                   (win_enable[2] ? 128 : 0);
        }
    }

    void ppu::io::control_reg::write(int offset, u8 value)
    {
        switch (offset)
        {
        case 0:
            mode = value & 3;
            cgb_mode = value & 8;
            frame_select = (value >> 4) & 1;
            hblank_oam_access = value & 32;
            one_dimensional = value & 64;
            forced_blank = value & 128;
            break;
        case 1:
            enable[0] = value & 1;
            enable[1] = value & 2;
            enable[2] = value & 4;
            enable[3] = value & 8;
            enable[4] = value & 16;
            win_enable[0] = value & 32;
            win_enable[1] = value & 64;
            win_enable[2] = value & 128;
            break;
        }
    }

    void ppu::io::status_reg::reset()
    {
        vblank_flag = false;
        hblank_flag = false;
        vcount_flag = false;
        vblank_interrupt = false;
        hblank_interrupt = false;
        vcount_interrupt = false;
        vcount_setting = 0;
    }

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

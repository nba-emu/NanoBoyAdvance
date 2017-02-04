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
    void ppu::io::dispcnt_reg::reset()
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

    auto ppu::io::dispcnt_reg::read(int offset) -> u8
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

    void ppu::io::dispcnt_reg::write(int offset, u8 value)
    {
        switch (offset)
        {
        case 0:
            mode = value & 7;
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

    void ppu::io::dispstat_reg::reset()
    {
        vblank_flag = false;
        hblank_flag = false;
        vcount_flag = false;
        vblank_interrupt = false;
        hblank_interrupt = false;
        vcount_interrupt = false;
        vcount_setting = 0;
    }

    auto ppu::io::dispstat_reg::read(int offset) -> u8
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

    void ppu::io::dispstat_reg::write(int offset, u8 value)
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

    void ppu::io::bgxcnt_reg::reset()
    {
        priority = 0;
        tile_block = 0;
        mosaic_enable = false;
        full_palette = false;
        map_block = 0;
        wraparound = false;
        screen_size = 0;
    }

    auto ppu::io::bgxcnt_reg::read(int offset) -> u8
    {
        switch (offset)
        {
        case 0:
            return priority |
                   (tile_block << 2) |
                   (mosaic_enable ? 64 : 0) |
                   (full_palette ? 128 : 0);
        case 1:
            return map_block |
                   (wraparound ? 32 : 0) |
                   (screen_size << 6);
        }
    }

    void ppu::io::bgxcnt_reg::write(int offset, u8 value)
    {
        switch (offset)
        {
        case 0:
            priority      = value & 3;
            tile_block    = (value >> 2) & 3;
            mosaic_enable = value & 64;
            full_palette  = value & 128;
            break;
        case 1:
            map_block   = value & 0x1F;
            wraparound  = value & 32;
            screen_size = value >> 6;
            break;
        }
    }

    void ppu::io::bgref_reg::reset()
    {
        value = internal = 0;
    }

    void ppu::io::bgref_reg::write(int offset, u8 value)
    {
        switch (offset)
        {
        case 0: value = (value & 0xFFFFFF00) | (value << 0); break;
        case 1: value = (value & 0xFFFF00FF) | (value << 8); break;
        case 2: value = (value & 0xFF00FFFF) | (value << 16); break;
        case 3: value = (value & 0x00FFFFFF) | (value << 24); break;
        }
        internal = ppu::decode_float32(value);
    }
}

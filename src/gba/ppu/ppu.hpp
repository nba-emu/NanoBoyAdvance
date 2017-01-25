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

#pragma once

#include "util/integer.hpp"

namespace gba
{
    /*struct dispstat
    {
        bool vblank_flag;
        bool hblank_flag;
        bool vcount_flag;
        bool vblank_interrupt;
        bool hblank_interrupt;
        bool vcount_interrupt;
        int vcount_setting;
    };*/

    class ppu
    {
    private:
        u8* m_pal;
        u8* m_oam;
        u8* m_vram;
        /*int m_vcount;
        struct dispcnt m_dispcnt;
        struct dispstat m_dispstat;*/

        #include "io.hpp"

    public:
        ppu();

        void reset();

        void set_memory(u8* pal, u8* oam, u8* vram);

        /*u8 read_dispstat_low()
        {
            return (m_dispstat.vblank_flag ? 1 : 0) |
                   (m_dispstat.hblank_flag ? 2 : 0) |
                   (m_dispstat.vcount_flag ? 4 : 0) |
                   (m_dispstat.vblank_interrupt ? 8 : 0) |
                   (m_dispstat.hblank_interrupt ? 16 : 0) |
                   (m_dispstat.vcount_interrupt ? 32 : 0);
        }

        u8 read_dispstat_high()
        {
            return m_dispstat.vcount_setting;
        }

        void write_dispstat_low(u8 value)
        {
            m_dispstat.vblank_flag = value & 1;
            m_dispstat.hblank_flag = value & 2;
            m_dispstat.vcount_flag = value & 4;
            m_dispstat.vblank_interrupt = value & 8;
            m_dispstat.hblank_interrupt = value & 16;
            m_dispstat.vcount_interrupt = value & 32;
        }

        void write_dispstat_high(u8 value)
        {
            m_dispstat.vcount_setting = value;
        }*/
    };
}

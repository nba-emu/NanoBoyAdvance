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

#ifdef PPU_INCLUDE

struct io
{
    struct dispcnt_reg
    {
        int mode;
        bool cgb_mode;
        int frame_select;
        bool hblank_oam_access;
        bool one_dimensional;
        bool forced_blank;
        bool enable[5];
        bool win_enable[3];

        void reset();
        auto read(int offset) -> u8;
        void write(int offset, u8 value);
    } control;

    struct dispstat_reg
    {
        bool vblank_flag;
        bool hblank_flag;
        bool vcount_flag;
        bool vblank_interrupt;
        bool hblank_interrupt;
        bool vcount_interrupt;
        int vcount_setting;

        void reset();
        auto read(int offset) -> u8;
        void write(int offset, u8 value);
    } status;

    int vcount;

    struct bgxcnt_reg
    {
        int priority;
        int tile_block;
        bool mosaic_enable;
        bool full_palette;//eh
        int map_block;
        bool wraparound;
        int screen_size;

        void reset();
        auto read(int offset) -> u8;
        void write(int offset, u8 value);
    } bgcnt[4];

    u16 bghofs[4];
    u16 bgvofs[4];

    struct bgref_reg
    {
        u32 value;
        float internal;

        void reset();
        void write(int offset, u8 value);
    } bgx[2], bgy[2];

    u16 bgpa[2];
    u16 bgpb[2];
    u16 bgpc[2];
    u16 bgpd[2];
} m_io;

#endif

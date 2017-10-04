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

#ifdef PPU_INCLUDE

struct IO {

    struct DisplayControl {
        int  mode;
        bool cgb_mode;
        int  frame_select;
        bool hblank_oam_access;
        bool one_dimensional;
        bool forced_blank;
        bool enable[5];
        bool win_enable[3];

        void reset();
        auto read(int offset) -> u8;
        void write(int offset, u8 value);
    } control;

    struct DisplayStatus {
        bool vblank_flag;
        bool hblank_flag;
        bool vcount_flag;
        bool vblank_interrupt;
        bool hblank_interrupt;
        bool vcount_interrupt;
        int  vcount_setting;

        void reset();
        auto read(int offset) -> u8;
        void write(int offset, u8 value);
    } status;

    int vcount;

    struct BackgroundControl {
        int  priority;
        int  tile_block;
        bool mosaic_enable;
        bool full_palette;//eh
        int  map_block;
        bool wraparound;
        int  screen_size;

        void reset();
        auto read(int offset) -> u8;
        void write(int offset, u8 value);
    } bgcnt[4];

    // horizontal and vertical scrolling info for each BG.
    u16 bghofs[4];
    u16 bgvofs[4];

    struct ReferencePoint {
        u32   value;
        float internal;

        void reset();
        void write(int offset, u8 value);
    } bgx[2], bgy[2];

    // rotate/scale parameters, PA/PB/PC/PD
    u16 bgpa[2];
    u16 bgpb[2];
    u16 bgpc[2];
    u16 bgpd[2];

    struct Mosaic {
        struct {
            int h;
            int v;
        } bg, obj;

        void reset();
        void write(int offset, u8 value);
    } mosaic;

    struct BlendControl {
        SpecialEffect sfx;
        bool targets[2][6];

        void reset();
        auto read(int offset) -> u8;
        void write(int offset, u8 value);
    } bldcnt;

    struct BlendAlpha {
        int eva;
        int evb;

        void reset();
        void write(int offset, u8 value);
    } bldalpha;

    struct BlendY {
        int evy;

        void reset();
        void write(u8 value);
    } bldy;

    struct WindowRange {
        int  min;
        int  max;
        bool changed; // only for internal optimization

        void reset();
        void write(int offset, u8 value);
    } winh[2], winv[2];

    struct WindowLayerSelect {
        bool enable[2][6];

        void reset();
        auto read(int offset) -> u8;
        void write(int offset, u8 value);

    } winin, winout;
} regs;

#endif

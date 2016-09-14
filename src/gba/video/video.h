///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
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


#ifndef __NBA_VIDEO_H__
#define __NBA_VIDEO_H__


#include "common/types.h"
#include "common/log.h"
#include "../interrupt.h"


namespace NanoboyAdvance
{
    struct Background
    {
        bool enable {false};
        bool mosaic {false};
        bool true_color {false};
        bool wraparound {false};
        int priority {0};
        int size {0};
        u32 tile_base {0};
        u32 map_base {0};
        u32 x {0};
        u32 y {0};
        u32 x_ref {0};
        u32 y_ref {0};
        float x_ref_int {0};
        float y_ref_int {0};
        u16 pa {0};
        u16 pb {0};
        u16 pc {0};
        u16 pd {0};
    };

    struct Object
    {
        bool enable {false};
        bool hblank_access {false};
        bool two_dimensional {false};
    };

    struct Window
    {
        bool enable {false};
        bool bg_in[4] {false, false, false, false};
        bool obj_in {false};
        bool sfx_in {false};
        u16 left {0};
        u16 right {0};
        u16 top {0};
        u16 bottom {0};
    };

    struct WindowOuter
    {
        bool bg[4] {false, false, false, false};
        bool obj {false};
        bool sfx {false};
    };

    struct ObjectWindow
    {
        bool enable {false};
        // TODO...
    };

    struct SpecialEffect
    {
        bool bg_select[2][4] {{false, false, false, false},
                              {false, false, false, false}};
        bool obj_select[2] {false, false};
        bool bd_select[2] {false, false};

        enum Effect
        {
            SFX_NONE = 0,
            SFX_ALPHABLEND = 1,
            SFX_INCREASE = 2,
            SFX_DECREASE = 3
        } effect {SFX_NONE};

        int eva {0};
        int evb {0};
        int evy {0};
    };

    class Video
    {   
    private:
        static const int VBLANK_INTERRUPT;
        static const int HBLANK_INTERRUPT;
        static const int VCOUNT_INTERRUPT;
        static const int EVENT_WAIT_CYCLES[3];
        static const u16 COLOR_TRANSPARENT;

        enum SpriteShape
        {
            SPRITE_SQUARE          = 0,
            SPRITE_HORIZONTAL      = 1,
            SPRITE_VERTICAL        = 2,
            SPRITE_PROHIBITED      = 3
        };

    public:
        enum RenderingPhase
        {
            PHASE_SCANLINE = 0,
            PHASE_HBLANK = 1,
            PHASE_VBLANK = 2
        };

        void Init(Interrupt* m_Interrupt);

        void Step();
        void Render();
        static float DecodeGBAFloat32(u32 number);

    private:
        static float DecodeGBAFloat16(u16 number);
        static u32 DecodeRGB555(u16 color);
        u16* DecodeTileLine4BPP(u32 block_base, u32 palette_base, int number, int line);
        u16* DecodeTileLine8BPP(u32 block_base, int number, int line, bool sprite);
        u16 DecodeTilePixel8BPP(u32 block_base, int number, int line, int column, bool sprite);
        bool IsVisible(int i, bool inside[3], bool outside);
        void RenderTextModeBG(int id);
        void RenderRotateScaleBG(int id);
        void RenderOAM(u32 tile_base);
        void ApplySFX(u16* target1, u16 target2);
        void ComposeScanline();

        ///////////////////////////////////////////////////////////
        // Class members
        //
        ///////////////////////////////////////////////////////////
        Interrupt* m_Interrupt;

        ///////////////////////////////////////////////////////////
        // Class members (Buffers)
        //
        ///////////////////////////////////////////////////////////
        u16 m_BgBuffer[4][240];
        u16 m_ObjBuffer[4][240];
        u8 m_WinMask[2][240];

    public:
        ///////////////////////////////////////////////////////////
        // Class members (IO)
        //
        ///////////////////////////////////////////////////////////
        RenderingPhase m_State       {PHASE_SCANLINE};
        Background m_BG[4];
        Object m_Obj;
        Window m_Win[2];
        WindowOuter m_WinOut;
        ObjectWindow m_ObjWin;
        SpecialEffect m_SFX;
        int m_VideoMode {0};
        u16 m_VCount {0};
        u8 m_VCountSetting          {0};
        bool m_FrameSelect          {false};
        bool m_ForcedBlank          {false};
        bool m_VCountFlag           {false};
        bool m_VBlankIRQ            {false};
        bool m_HBlankIRQ            {false};
        bool m_VCountIRQ            {false};

        ///////////////////////////////////////////////////////////
        // Class members (Memory)
        //
        ///////////////////////////////////////////////////////////
        u8 m_PAL[0x400];
        u8 m_VRAM[0x18000];
        u8 m_OAM[0x400];

        ///////////////////////////////////////////////////////////
        // Class members (misc.)
        //
        ///////////////////////////////////////////////////////////
        bool m_HBlankDMA            {false};
        bool m_VBlankDMA            {false};
        bool m_RenderScanline       {false};
        int  m_WaitCycles           { EVENT_WAIT_CYCLES[0] };
        u32 m_OutputBuffer[240 * 160];
    };
}


#include "video.inl"


#endif  // __NBA_VIDEO_H__

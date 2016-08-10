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


#include "util/types.h"
#include "util/log.h"
#include "interrupt.h"
#include "video_structs.h"
#include "composer.h"
#include <cstring>


namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \file    video.h
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \class   GBAVideo
    /// \brief   Provides methods for rendering several modes.
    ///
    ///////////////////////////////////////////////////////////
    class GBAVideo
    {   
    private:

        static const int VBLANK_INTERRUPT;
        static const int HBLANK_INTERRUPT;
        static const int VCOUNT_INTERRUPT;
        static const u16 COLOR_BACKDROP;

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   GBAVideoSpriteShape
        ///
        /// Defines all possible sprite shapes.
        ///
        ///////////////////////////////////////////////////////////
        enum class GBAVideoSpriteShape
        {
            Square          = 0,
            Horizontal      = 1,
            Vertical        = 2,
            Prohibited      = 3
        };

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   GBASpecialEffect
        ///
        /// Defines GBA Special Effects (SFX).
        ///
        ///////////////////////////////////////////////////////////
        enum class GBASpecialEffect
        {
            None            = 0,
            AlphaBlend      = 1,
            Increase        = 2,
            Decrease        = 3
        };

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      DecodeTileLine4BPP
        /// \brief   Decodes a single 4-bit tile line.
        ///
        /// \param    block_base    Address of block/tile data.
        /// \param    palette_base  Palette address.
        /// \param    number        Tile index number.
        /// \param    line          The line to decode.
        /// \returns  The decoded line.
        ///
        ///////////////////////////////////////////////////////////
        inline u16* DecodeTileLine4BPP(u32 block_base, u32 palette_base, int number, int line)
        {
            u16* data = new u16[8];
            u32 offset = block_base + number * 32 + line * 4;

            memset(data, 0, 16);

            for (int i = 0; i < 4; i++)
            {
                u8 value = m_VRAM[offset + i];
                int left_index = value & 0xF;
                int right_index = value >> 4;

                if (left_index != 0)
                    data[i * 2] = (m_PAL[palette_base + left_index * 2 + 1] << 8) |
                                   m_PAL[palette_base + left_index * 2];
                else
                    data[i * 2] = COLOR_BACKDROP;

                if (right_index != 0)
                    data[i * 2 + 1] = (m_PAL[palette_base + right_index * 2 + 1] << 8) |
                                       m_PAL[palette_base + right_index * 2];
                else
                    data[i * 2 + 1] = COLOR_BACKDROP;
            }

            return data;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      DecodeTileLine8BPP
        /// \brief   Decodes a single 8-bit tile line.
        ///
        /// \param    block_base  Address of block/tile data
        /// \param    number      Tile index number.
        /// \param    line        The line to decode.
        /// \param    sprite      Wether to use the sprite palette.
        /// \returns  The decoded line.
        ///
        ///////////////////////////////////////////////////////////
        inline u16* DecodeTileLine8BPP(u32 block_base, int number, int line, bool sprite)
        {
            u16* data = new u16[8];
            u32 offset = block_base + number * 64 + line * 8;
            u32 palette_base = sprite ? 0x200 : 0x0;

            memset(data, 0, 16);

            for (int i = 0; i < 8; i++)
            {
                u8 value = m_VRAM[offset + i];

                if (value == 0)
                {
                    data[i] = COLOR_BACKDROP;
                    continue;
                }

                data[i] = (m_PAL[palette_base + value * 2 + 1] << 8) |
                           m_PAL[palette_base + value * 2];
            }

            return data;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      DecodeTilePixel8BPP
        /// \brief   Decodes a single 8-bit tile pixel
        ///
        /// \param    block_base  Address of block/tile data
        /// \param    number      Tile index number.
        /// \param    line        Pixel y-Coordinate.
        /// \param    column      Pixel x-Coordinate.
        /// \param    sprite      Wether to use the sprite palette.
        /// \returns  The decoded pixel.
        ///
        ///////////////////////////////////////////////////////////
        inline u16 DecodeTilePixel8BPP(u32 block_base, int number, int line, int column, bool sprite)
        {
            u8 value = m_VRAM[block_base + number * 64 + line * 8 + column];
            u32 palette_base = sprite ? 0x200 : 0x0;

            if (value == 0)
                return COLOR_BACKDROP;

            return (m_PAL[palette_base + value * 2 + 1] << 8) |
                    m_PAL[palette_base + value * 2];
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      RenderBackgroundMode0
        /// \brief   Performs rendering in BG mode 0.
        ///
        /// \param  id  ID of the background to render.
        ///
        ///////////////////////////////////////////////////////////
        void RenderBackgroundMode0(int id);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      RenderBackgroundMode1
        /// \brief   Performs rendering in BG mode 1.
        ///
        /// \param  id  ID of the background to render.
        ///
        ///////////////////////////////////////////////////////////
        void RenderBackgroundMode1(int id);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      RenderSprites
        /// \brief   Performs rendering of OAM sprites.
        ///
        /// \param  tile_base  Address of block/tile data.
        ///
        ///////////////////////////////////////////////////////////
        void RenderSprites(u32 tile_base);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      DecodeGBAFloat16
        /// \brief   Decodes the GBAFloat16 format to native float.
        ///
        /// \param    number  The GBAFloat16 value to decode.
        /// \returns  The native float value.
        ///
        ///////////////////////////////////////////////////////////
        static inline float DecodeGBAFloat16(u16 number)
        {
            bool is_negative = number & (1 << 15);
            s32 int_part = (number >> 8) | (is_negative ? 0xFFFFFF00 : 0);
            float frac_part = static_cast<float>(number & 0xFF) / 256;

            return static_cast<float>(int_part) + (is_negative ? -frac_part : frac_part);        
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      EncodeGBAFloat32
        /// \brief   Encodes a native float into GBAFloat32 format.
        ///
        /// \param    The native float to encode.
        /// \returns  The encoded GBAFloat32 value.
        ///
        ///////////////////////////////////////////////////////////
        static inline u32 EncodeGBAFloat32(float number)
        {
            s32 int_part = static_cast<s32>(number);
            u8 frac_part = static_cast<u8>((number - int_part) * (number >= 0 ? 256 : -256)); // optimize

            return (u32)(int_part << 8 | frac_part);
        }

    public:

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   GBAVideoState
        ///
        /// Defines all phases of video rendering.
        ///
        ///////////////////////////////////////////////////////////
        enum class GBAVideoState
        {
            Scanline,
            HBlank,
            VBlank
        };

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      DecodeGBAFloat32
        /// \brief   Decodes the GBAFloat32 format to native float.
        ///
        /// \param    number  The GBAFloat32 value to decode.
        /// \returns  The native float value.
        ///
        ///////////////////////////////////////////////////////////
        static inline float DecodeGBAFloat32(u32 number)
        {
            bool is_negative = number & (1 << 27);
            s32 int_part = ((number & ~0xF0000000) >> 8) | (is_negative ? 0xFFF00000 : 0);
            float frac_part = static_cast<float>(number & 0xFF) / 256;

            return static_cast<float>(int_part) + (is_negative ? -frac_part : frac_part);
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Constructor
        ///
        ///////////////////////////////////////////////////////////
        GBAVideo(GBAInterrupt* m_Interrupt);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Render
        ///
        /// Renders the current scanline.
        ///
        ///////////////////////////////////////////////////////////
        void Render();

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Step
        ///
        /// Updates PPU/Video state.
        ///
        ///////////////////////////////////////////////////////////
        void Step();

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 9th, 2016
        /// \fn      SetupComposer
        ///
        /// Setups a GBAComposer derived object for composing.
        ///
        /// \param  composer  The composer.
        ///
        ///////////////////////////////////////////////////////////
        void SetupComposer(GBAComposer* composer);

    private:

        ///////////////////////////////////////////////////////////
        // Class members
        //
        ///////////////////////////////////////////////////////////
        GBAInterrupt* m_Interrupt;
        int m_Ticks {0};

        ///////////////////////////////////////////////////////////
        // Class members (Buffers)
        //
        ///////////////////////////////////////////////////////////
        u16 m_BgBuffer[4][240];
        u16 m_ObjBuffer[4][240];

    public:

        ///////////////////////////////////////////////////////////
        // Class members (IO)
        //
        ///////////////////////////////////////////////////////////
        GBAVideoState m_State       {GBAVideoState::Scanline};
        Background m_BG[4];
        Object m_Obj;
        Window m_Win[2];
        WindowOuter m_WinOut;
        ObjectWindow m_ObjWin;
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
        // Class members (misc. flags)
        //
        ///////////////////////////////////////////////////////////
        bool m_HBlankDMA            {false};
        bool m_VBlankDMA            {false};
        bool m_RenderScanline       {false};
    };
}


#endif  // __NBA_VIDEO_H__

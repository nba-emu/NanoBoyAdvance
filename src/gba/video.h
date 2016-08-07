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

    public:
        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \struct  Background
        ///
        /// Describes one Background.
        ///
        ///////////////////////////////////////////////////////////
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
            u16 x_ref {0};
            u16 y_ref {0};
            u16 x_ref_int {0};
            u16 y_ref_int {0};
            u16 pa {0};
            u16 pb {0};
            u16 pc {0};
            u16 pd {0};
        };

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \struct  Object
        ///
        /// Holds OBJ-system settings.
        ///
        ///////////////////////////////////////////////////////////
        struct Object
        {
            bool enable {false};
            bool hblank_access {false};
            bool two_dimensional {false};
        };

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \struct  Window
        ///
        /// Defines one Window.
        ///
        ///////////////////////////////////////////////////////////
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

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \struct  WindowOuter
        ///
        /// Defines the outer area of windows.
        ///
        ///////////////////////////////////////////////////////////
        struct WindowOuter
        {
            bool bg[4] {false, false, false, false};
            bool obj {false};
            bool sfx {false};
        };

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \struct  ObjectWindow
        ///
        /// Defines the object window.
        ///
        ///////////////////////////////////////////////////////////
        struct ObjectWindow
        {
            bool enable {false};
            // TODO...
        };

    private:
        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      DecodeRGB5
        /// \brief   Decodes GBA RGB555 to ARGB32 format.
        ///
        /// \param    color  RGB555 color value to decode.
        /// \returns  The decoded ARGB32 value.
        ///
        ///////////////////////////////////////////////////////////
        inline u32 DecodeRGB5(u16 color)
        {
            return 0xFF000000 |
                   (((color & 0x1F) * 8) << 16) |
                   ((((color >> 5) & 0x1F) * 8) << 8) |
                   (((color >> 10) & 0x1F) * 8);
        }

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
        /// \returns  The decoded line in ARGB32 format.
        ///
        ///////////////////////////////////////////////////////////
        inline u32* DecodeTileLine4BPP(u32 block_base, u32 palette_base, int number, int line)
        {
            u32 offset = block_base + number * 32 + line * 4;
            u32* data = new u32[8];

            // Empty buffer.
            memset(data, 0, 32);

            for (int i = 0; i < 4; i++)
            {
                u8 value = m_VRAM[offset + i];
                int left_index = value & 0xF;
                int right_index = value >> 4;
                u32 left_color = DecodeRGB5((m_PAL[palette_base + left_index * 2 + 1] << 8) |
                                             m_PAL[palette_base + left_index * 2]);
                u32 right_color = DecodeRGB5((m_PAL[palette_base + right_index * 2 + 1] << 8) |
                                              m_PAL[palette_base + right_index * 2]);

                // Copy left and right pixel to buffer
                data[i * 2] = (left_index == 0) ? (left_color & ~0xFF000000) : left_color;
                data[i * 2 + 1] = (right_index == 0) ? (right_color & ~0xFF000000) : right_color;
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
        /// \returns  The decoded line in ARGB32 format.
        ///
        ///////////////////////////////////////////////////////////
        inline u32* DecodeTileLine8BPP(u32 block_base, int number, int line, bool sprite)
        {
            u32 offset = block_base + number * 64 + line * 8;
            u32 palette_base = sprite ? 0x200 : 0x0;
            u32* data = new u32[8];

            // Empty buffer.
            memset(data, 0, 32);

            for (int i = 0; i < 8; i++)
            {
                u8 value = m_VRAM[offset + i];
                u32 color = DecodeRGB5((m_PAL[palette_base + value * 2 + 1] << 8) |
                                        m_PAL[palette_base + value * 2]);

                data[i] = (value == 0) ? (color & ~0xFF000000) : color;
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
        /// \returns  The decoded pixel in ARGB32 pixel.
        ///
        ///////////////////////////////////////////////////////////
        inline u32 DecodeTilePixel8BPP(u32 block_base, int number, int line, int column, bool sprite)
        {
            u8 value = m_VRAM[block_base + number * 64 + line * 8 + column];
            u32 palette_base = sprite ? 0x200 : 0x0;
            u32 color = DecodeRGB5((m_PAL[palette_base + value * 2 + 1] << 8) |
                                    m_PAL[palette_base + value * 2]);
            
            return (value == 0) ? (color & ~0xFF000000) : color;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      OverlayLineBuffers
        /// \brief   Copies a source buffer on the destination buffer.
        ///
        /// \param  dst  Destination Buffer
        /// \param  src  Source Buffer
        ///
        ///////////////////////////////////////////////////////////
        inline void OverlayLineBuffers(u32* dst, u32* src)
        {
            for (int i = 0; i < 240; i++) 
            {
                u32 color = src[i];

                if ((color >> 24) != 0)
                    dst[i] = color | 0xFF000000;
            }
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      DrawLineToBuffer
        /// \brief   Draws a line to the screen buffer
        ///
        /// \param  line_buffer  The line to draw.
        /// \param  backdrop     Disables transparency.
        ///
        ///////////////////////////////////////////////////////////
        inline void DrawLineToBuffer(u32* line_buffer, bool backdrop)
        {
            for (int i = 0; i < 240; i++)
            {
                u32 color = line_buffer[i];

                if (backdrop || (color >> 24) != 0)
                    m_Buffer[m_VCount * 240 + i] = line_buffer[i] | 0xFF000000;
            }
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
        /// \param  priority   Priority filter.
        /// \param  tile_base  Address of block/tile data.
        ///
        ///////////////////////////////////////////////////////////
        void RenderSprites(int priority, u32 tile_base);

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
        /// \fn      Step
        ///
        /// Updates PPU/Video state.
        ///
        ///////////////////////////////////////////////////////////
        void Step();

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Render
        ///
        /// Renders the current scanline.
        ///
        ///////////////////////////////////////////////////////////
        void Render();

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
        u32 m_BdBuffer[240];
        u32 m_BgBuffer[4][240];
        u32 m_ObjBuffer[4][240];

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

        ///////////////////////////////////////////////////////////
        // Class members (Buffers)
        //
        ///////////////////////////////////////////////////////////
        u32 m_Buffer[240 * 160];
    };
}


#endif  // __NBA_VIDEO_H__

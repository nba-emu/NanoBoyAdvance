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


#include "video.h"


namespace NanoboyAdvance
{
    const int Video::VBLANK_INTERRUPT = 1;
    const int Video::HBLANK_INTERRUPT = 2;
    const int Video::VCOUNT_INTERRUPT = 4;
    const int Video::EVENT_WAIT_CYCLES[3] = { 960, 272, 1232 };

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      Constructor
    ///
    ///////////////////////////////////////////////////////////
    void Video::Init(Interrupt* interrupt)
    {
        // Assign interrupt struct to video device
        this->m_Interrupt = interrupt;

        // Zero init memory buffers
        memset(m_PAL, 0, 0x400);
        memset(m_VRAM, 0, 0x18000);
        memset(m_OAM, 0, 0x400);
    }


    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      RenderTextModeBG
    ///
    ///////////////////////////////////////////////////////////
    void Video::RenderTextModeBG(int id)
    {
        struct Background bg = this->m_BG[id];
        u16* buffer = m_BgBuffer[id];

        int width = ((bg.size & 1) + 1) * 256;
        int height = ((bg.size >> 1) + 1) * 256;
        int y_scrolled = (m_VCount + bg.y) % height;
        int row = y_scrolled / 8;
        int row_rmdr = y_scrolled % 8;
        int left_area = 0;
        int right_area = 1;
        u16 line_buffer[width];
        u32 offset;

        if (row >= 32)
        {
            left_area = (bg.size & 1) + 1;
            right_area = 3;
            row -= 32;
        }

        offset = bg.map_base + left_area * 0x800 + 64 * row;

        for (int x = 0; x < width / 8; x++)
        {
            u16 tile_encoder = (m_VRAM[offset + 1] << 8) | 
                                m_VRAM[offset];
            int tile_number = tile_encoder & 0x3FF;
            bool horizontal_flip = tile_encoder & (1 << 10);
            bool vertical_flip = tile_encoder & (1 << 11); 
            int row_rmdr_flip = row_rmdr;
            u16* tile_data;

            if (vertical_flip)
                row_rmdr_flip = 7 - row_rmdr_flip;

            if (bg.true_color)
            {
                tile_data = DecodeTileLine8BPP(bg.tile_base, tile_number, row_rmdr_flip, false);
            } else 
            {
                int palette = tile_encoder >> 12;
                tile_data = DecodeTileLine4BPP(bg.tile_base, palette * 0x20, tile_number, row_rmdr_flip);
            }

            if (horizontal_flip)
                for (int i = 0; i < 8; i++)
                    line_buffer[x * 8 + i] = tile_data[7 - i];
            else
                for (int i = 0; i < 8; i++)
                    line_buffer[x * 8 + i] = tile_data[i];

            delete[] tile_data;
            
            if (x == 31) 
                offset = bg.map_base + right_area * 0x800 + 64 * row;
            else
                offset += 2;
        }

        for (int i = 0; i < 240; i++)
            buffer[i] = line_buffer[(bg.x + i) % width];
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      RenderRotateScaleBG
    ///
    ///////////////////////////////////////////////////////////
    void Video::RenderRotateScaleBG(int id)
    {
        u32 tile_base = m_BG[id].tile_base;
        u32 map_base = m_BG[id].map_base;
        bool wraparound = m_BG[id].wraparound;
        float parameter_a = DecodeGBAFloat16(m_BG[id].pa);
        float parameter_c = DecodeGBAFloat16(m_BG[id].pc);
        float x_ref = m_BG[id].x_ref_int;
        float y_ref = m_BG[id].y_ref_int;
        u16* buffer = m_BgBuffer[id];
        int size, block_width;

        // Decode size and block width of the map.
        switch (m_BG[id].size)
        {
        case 0: size = 128; block_width = 16; break;
        case 1: size = 256; block_width = 32; break;
        case 2: size = 512; block_width = 64; break;
        case 3: size = 1024; block_width = 128; break;
        }

        // Draw exactly one line.
        for (int i = 0; i < 240; i++)
        {
            int x = x_ref + i * parameter_a;
            int y = y_ref + i * parameter_c;
            bool is_backdrop = false;

            // Handles x-Coord over-/underflow
            if (x >= size)
            {
                if (wraparound) x = x % size;
                else is_backdrop = true;
            }
            else if (x < 0)
            {
                if (wraparound) x = (size + x) % size;
                else is_backdrop = true;
            }

            // Handles y-Coord over-/underflow
            if (y >= size)
            {
                if (wraparound) y = y % size;
                else is_backdrop = true;
            }
            else if (y < 0)
            {
                if (wraparound) y = (size + y) % size;
                else is_backdrop = true;
            }

            // Handles empty spots.
            if (is_backdrop)
            {
                buffer[i] = COLOR_TRANSPARENT;
                continue;
            }

            // Raster-position of the tile.
            int map_x = x / 8;
            int map_y = y / 8;

            // Position of the wanted pixel inside the tile.
            int tile_x = x % 8;
            int tile_y = y % 8;

            // Get the tile number from the map using the raster-position
            int tile_number = m_VRAM[map_base + map_y * block_width + map_x];

            // Get the wanted pixel.
            buffer[i] = DecodeTilePixel8BPP(tile_base, tile_number, tile_y, tile_x, false);
        }
    }
       
    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      RenderOAM
    ///
    ///////////////////////////////////////////////////////////
    void Video::RenderOAM(u32 tile_base)
    {
        // Process OBJ127 first, because OBJ0 has highest priority (OBJ0 overlays OBJ127, not vice versa)
        u32 offset = 127 * 8;

        // Walk all entries
        for (int i = 0; i < 128; i++)
        {
            u16 attribute0 = (m_OAM[offset + 1] << 8) | m_OAM[offset];
            u16 attribute1 = (m_OAM[offset + 3] << 8) | m_OAM[offset + 2];
            u16 attribute2 = (m_OAM[offset + 5] << 8) | m_OAM[offset + 4];

            // Only render those which have matching priority
            //if (((attribute2 >> 10) & 3) == priority)
            {
                int width;
                int height;
                int x = attribute1 & 0x1FF;
                int y = attribute0 & 0xFF;
                SpriteShape shape = static_cast<SpriteShape>(attribute0 >> 14);
                int size = attribute1 >> 14;
                int priority = (attribute2 >> 10) & 3;

                // Decode width and height
                switch (shape)
                {
                case SPRITE_SQUARE:
                    switch (size)
                    {
                    case 0: width = 8; height = 8; break;
                    case 1: width = 16; height = 16; break;
                    case 2: width = 32; height = 32; break;
                    case 3: width = 64; height = 64; break;
                    }
                    break;
                case SPRITE_HORIZONTAL:
                    switch (size)
                    {
                    case 0: width = 16; height = 8; break;
                    case 1: width = 32; height = 8; break;
                    case 2: width = 32; height = 16; break;
                    case 3: width = 64; height = 32; break;
                    }
                    break;
                case SPRITE_VERTICAL:
                    switch (size)
                    {
                    case 0: width = 8; height = 16; break;
                    case 1: width = 8; height = 32; break;
                    case 2: width = 16; height = 32; break;
                    case 3: width = 32; height = 64; break;
                    }
                    break;
                case SPRITE_PROHIBITED:
                    width = 0;
                    height = 0;
                    break;
                }

                // Determine if there is something to render for this sprite
                if (m_VCount >= y && m_VCount <= y + height - 1)
                {
                    int internal_line = m_VCount - y;
                    int displacement_y;
                    int row;
                    int tiles_per_row = width / 8;
                    int tile_number = attribute2 & 0x3FF;
                    int palette_number = attribute2 >> 12;
                    bool rotate_scale = attribute0 & (1 << 8) ? true : false;
                    bool horizontal_flip = !rotate_scale && (attribute1 & (1 << 12));
                    bool vertical_flip = !rotate_scale && (attribute1 & (1 << 13));
                    bool color_mode = attribute0 & (1 << 13) ? true : false;
                    
                    // It seems like the tile number is divided by two in 256 color mode
                    // Though we should check this because I cannot find information about this in GBATEK
                    if (color_mode)
                    {
                        tile_number /= 2;
                    }

                    // Apply (outer) vertical flip if required
                    if (vertical_flip) 
                    {
                        internal_line = height - internal_line;                    
                    }

                    // Calculate some stuff
                    displacement_y = internal_line % 8;
                    row = (internal_line - displacement_y) / 8;

                    // Apply (inner) vertical flip
                    if (vertical_flip)
                    {
                        displacement_y = 7 - displacement_y;
                        row = (height / 8) - row;
                    }

                    // Render all visible tiles of the sprite
                    for (int j = 0; j < tiles_per_row; j++)
                    {
                        int current_tile_number;
                        u16* tile_data;

                        // Determine the tile to render
                        if (m_Obj.two_dimensional)
                        {
                            current_tile_number = tile_number + row * tiles_per_row + j;
                        }
                        else
                        {
                            current_tile_number = tile_number + row * 32 + j;
                        }

                        // Render either in 256 colors or 16 colors mode
                        if (color_mode)
                        {
                            // 256 colors
                            tile_data = DecodeTileLine8BPP(tile_base, current_tile_number, displacement_y, true);
                        }
                        else
                        {
                            // 16 colors (use palette_nummer)
                            tile_data = DecodeTileLine4BPP(tile_base, 0x200 + palette_number * 0x20, current_tile_number, displacement_y);
                        }

                        // Copy data
                        if (horizontal_flip) // not very beautiful but faster
                        {
                            for (int k = 0; k < 8; k++)
                            {
                                int dst_index = x + (tiles_per_row - j - 1) * 8 + (7 - k);
                                u16 color = tile_data[k];

                                if (color != COLOR_TRANSPARENT && dst_index < 240)
                                {
                                    m_ObjBuffer[priority][dst_index] = color;
                                }
                            }

                        }
                        else
                        {
                            for (int k = 0; k < 8; k++)
                            {
                                int dst_index = x + j * 8 + k;
                                u16 color = tile_data[k];

                                if (color != COLOR_TRANSPARENT && dst_index < 240)
                                {
                                    m_ObjBuffer[priority][dst_index] = tile_data[k];
                                }
                            }
                        }

                        // We don't need that memory anymore
                        delete[] tile_data;
                    }
                }
            }

            // Update offset to the next entry
            offset -= 8;
        }
    }    

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      Render
    ///
    ///////////////////////////////////////////////////////////
    void Video::Render()
    {
        // Reset obj buffers
        for (int i = 0; i < 240; i++)
        {
            m_ObjBuffer[0][i] = 0x8000;
            m_ObjBuffer[1][i] = 0x8000;
            m_ObjBuffer[2][i] = 0x8000;
            m_ObjBuffer[3][i] = 0x8000;
        }

        // Update window buffers
        for (int i = 0; i < 2; i++)
        {
            if (!m_Win[i].enable)
                continue;

            if (m_Win[i].top <= m_Win[i].bottom &&
                (m_VCount < m_Win[i].top ||
                 m_VCount >= m_Win[i].bottom))
            {
                memset(m_WinMask[i], 0, 240);
                continue;
            }

            if (m_Win[i].top > m_Win[i].bottom &&
                (m_VCount < m_Win[i].top &&
                 m_VCount >= m_Win[i].bottom))
            {
                memset(m_WinMask[i], 0, 240);
                continue;
            }

            if (m_Win[i].left <= m_Win[i].right + 1)
            {
                for (int j = 0; j < 240; j++)
                {
                    if (j >= m_Win[i].left && j < m_Win[i].right)
                        m_WinMask[i][j] = 1;
                    else
                        m_WinMask[i][j] = 0;
                }
            }
            else
            {
                for (int j = 0; j < 240; j++)
                {
                    if (j < m_Win[i].right || j >= m_Win[i].left)
                        m_WinMask[i][j] = 1;
                    else
                        m_WinMask[i][j] = 0;
                }
            }
        }

        // Call mode specific rendering logic
        switch (m_VideoMode)
        {
        case 0:
        {
            // BG Mode 0 - 240x160 pixels, Text mode
            for (int i = 0; i < 4; i++)
            {
                if (m_BG[i].enable)
                    RenderTextModeBG(i);
            }
            break;
        }
        case 1:
        {        
            // BG Mode 1 - 240x160 pixels, Text and RS mode mixed
            if (m_BG[0].enable)
                RenderTextModeBG(0);

            if (m_BG[1].enable)
                RenderTextModeBG(1);

            if (m_BG[2].enable)
                RenderRotateScaleBG(2);

            break;
        }
        case 2:
        {
            // BG Mode 2 - 240x160 pixels, RS mode
            if (m_BG[2].enable)
                RenderRotateScaleBG(2);

            if (m_BG[3].enable)
                RenderRotateScaleBG(3);

            break;
        }
        case 3:
            // BG Mode 3 - 240x160 pixels, 32768 colors
            if (m_BG[2].enable)
            {
                u32 offset = m_VCount * 240 * 2;

                for (int x = 0; x < 240; x++)
                {
                    m_BgBuffer[2][x] = (m_VRAM[offset + 1] << 8) | m_VRAM[offset];
                    offset += 2;
                }
            }
            break;
        case 4:
            // BG Mode 4 - 240x160 pixels, 256 colors (out of 32768 colors)
            if (m_BG[2].enable)
            {
                u32 page = m_FrameSelect ? 0xA000 : 0;

                for (int x = 0; x < 240; x++)
                {
                    u8 index = m_VRAM[page + m_VCount * 240 + x];
                    m_BgBuffer[2][x] = m_PAL[index * 2] | (m_PAL[index * 2 + 1] << 8);
                }
            }
            break;
        case 5:
            // BG Mode 5 - 160x128 pixels, 32768 colors
            if (m_BG[2].enable)
            {
                u32 offset = (m_FrameSelect ? 0xA000 : 0) + m_VCount * 160 * 2;

                for (int x = 0; x < 240; x++)
                {
                    if (x < 160 && m_VCount < 128)
                    {
                        m_BgBuffer[2][x] = (m_VRAM[offset + 1] << 8) | m_VRAM[offset];
                        offset += 2;
                    }
                    else
                    {
                        m_BgBuffer[2][x] = 0x8000;
                    }
                }
            }
            break;
        }
        
        // Render sprites if enabled
        if (m_Obj.enable)
            RenderOAM(0x10000);
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      Step
    ///
    ///////////////////////////////////////////////////////////
    void NanoboyAdvance::Video::Step()
    {
        m_RenderScanline = false;
        m_VCountFlag = m_VCount == m_VCountSetting;

        switch (m_State)
        {
        case RenderingPhase::Scanline:
            m_HBlankDMA = true;
            m_State = RenderingPhase::HBlank;
            m_WaitCycles = EVENT_WAIT_CYCLES[1];

            m_BG[2].x_ref_int += DecodeGBAFloat16(m_BG[2].pb);
            m_BG[2].y_ref_int += DecodeGBAFloat16(m_BG[2].pd);
            m_BG[3].x_ref_int += DecodeGBAFloat16(m_BG[3].pb);
            m_BG[3].y_ref_int += DecodeGBAFloat16(m_BG[3].pd);
                
            if (m_HBlankIRQ)
                m_Interrupt->if_ |= HBLANK_INTERRUPT;

            m_RenderScanline = true;
            return;
        case RenderingPhase::HBlank:
            m_VCount++;

            if (m_VCountFlag && m_VCountIRQ)
                m_Interrupt->if_ |= VCOUNT_INTERRUPT;
                
            if (m_VCount == 160)
            {
                m_BG[2].x_ref_int = DecodeGBAFloat32(m_BG[2].x_ref);
                m_BG[2].y_ref_int = DecodeGBAFloat32(m_BG[2].y_ref);
                m_BG[3].x_ref_int = DecodeGBAFloat32(m_BG[3].x_ref);
                m_BG[3].y_ref_int = DecodeGBAFloat32(m_BG[3].y_ref);

                m_HBlankDMA = false;
                m_VBlankDMA = true;
                m_State = RenderingPhase::VBlank;
                m_WaitCycles = EVENT_WAIT_CYCLES[2];

                if (m_VBlankIRQ)
                    m_Interrupt->if_ |= VBLANK_INTERRUPT;
            }
            else
            {
                m_HBlankDMA = false;
                m_State = RenderingPhase::Scanline;
                m_WaitCycles = EVENT_WAIT_CYCLES[0];
            }
            return;
        case RenderingPhase::VBlank:
            m_VCount++;

            if (m_VCountFlag && m_VCountIRQ)
                m_Interrupt->if_ |= VCOUNT_INTERRUPT;

            if (m_VCount == 227)
            {
                m_VBlankDMA = false;
                m_State = RenderingPhase::Scanline;
                m_WaitCycles = EVENT_WAIT_CYCLES[0];
                m_VCount = 0;
            }
            else
            {
                m_WaitCycles = EVENT_WAIT_CYCLES[2];
            }
            return;
        }
    }

    void Video::SetupComposer(GBAComposer* composer)
    {
        composer->SetVerticalCounter(&m_VCount);
        composer->SetBackdropColor((u16*)&m_PAL);
        composer->SetObjectInfo(&m_Obj);

        for (int i = 0; i < 4; i++)
        {
            composer->SetBackgroundInfo(i, &m_BG[i]);
            composer->SetBackgroundBuffer(i, m_BgBuffer[i]);
            composer->SetObjectBuffer(i, m_ObjBuffer[i]);
        }

        composer->SetWindowMaskBuffer(0, m_WinMask[0]);
        composer->SetWindowMaskBuffer(1, m_WinMask[1]);
        composer->SetWindowInfo(0, &m_Win[0]);
        composer->SetWindowInfo(1, &m_Win[1]);
        composer->SetObjectWindowInfo(&m_ObjWin);
        composer->SetWindowOuterInfo(&m_WinOut);
        composer->SetSFXInfo(&m_SFX);
    }
}

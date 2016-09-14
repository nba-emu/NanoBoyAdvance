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


#include "../video.h"


namespace NanoboyAdvance
{
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
}

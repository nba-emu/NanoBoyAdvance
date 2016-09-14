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
#include <cstring>


namespace NanoboyAdvance
{
    const int Video::VBLANK_INTERRUPT = 1;
    const int Video::HBLANK_INTERRUPT = 2;
    const int Video::VCOUNT_INTERRUPT = 4;
    const int Video::EVENT_WAIT_CYCLES[3] = { 960, 272, 1232 };
    const u16 Video::COLOR_TRANSPARENT = 0x8000;

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

        ComposeScanline();
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
        case PHASE_SCANLINE:
            m_HBlankDMA = true;
            m_State = PHASE_HBLANK;
            m_WaitCycles = EVENT_WAIT_CYCLES[1];

            m_BG[2].x_ref_int += DecodeGBAFloat16(m_BG[2].pb);
            m_BG[2].y_ref_int += DecodeGBAFloat16(m_BG[2].pd);
            m_BG[3].x_ref_int += DecodeGBAFloat16(m_BG[3].pb);
            m_BG[3].y_ref_int += DecodeGBAFloat16(m_BG[3].pd);
                
            if (m_HBlankIRQ)
                m_Interrupt->if_ |= HBLANK_INTERRUPT;

            m_RenderScanline = true;
            return;
        case PHASE_HBLANK:
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
                m_State = PHASE_VBLANK;
                m_WaitCycles = EVENT_WAIT_CYCLES[2];

                if (m_VBlankIRQ)
                    m_Interrupt->if_ |= VBLANK_INTERRUPT;
            }
            else
            {
                m_HBlankDMA = false;
                m_State = PHASE_SCANLINE;
                m_WaitCycles = EVENT_WAIT_CYCLES[0];
            }
            return;
        case PHASE_VBLANK:
            m_VCount++;

            if (m_VCountFlag && m_VCountIRQ)
                m_Interrupt->if_ |= VCOUNT_INTERRUPT;

            if (m_VCount == 227)
            {
                m_VBlankDMA = false;
                m_State = PHASE_SCANLINE;
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
}

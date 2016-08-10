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


#include "soft_composer.h"
#include <cstring>


namespace NanoboyAdvance
{
    void GBASoftComposer::Update()
    {
        int line_start = *m_VCount * 240;

        // Update background buffers.
        for (int i = 0; i < 4; i++)
        {
            if (m_BG[i]->enable)
            {
                for (int j = 0; j < 240; j++)
                    m_BgFinalBuffer[i][line_start + j] = m_BgBuffer[i][j];
            }
        }

        // Update object (oam) buffers.
        if (m_Obj->enable)
        {
            for (int i = 0; i < 240; i++)
            {
                m_ObjFinalBuffer[0][line_start + i] = m_ObjBuffer[0][i];
                m_ObjFinalBuffer[1][line_start + i] = m_ObjBuffer[1][i];
                m_ObjFinalBuffer[2][line_start + i] = m_ObjBuffer[2][i];
                m_ObjFinalBuffer[3][line_start + i] = m_ObjBuffer[3][i];
            }
        }

        // Update window masks
        for (int i = 0; i < 2; i++)
        {
            if (m_Win[i]->enable)
            {
                for (int j = 0; j < 240; j++)
                    m_WinFinalMask[i][line_start + j] = m_WinMask[i][j];
            }
        }
    }

    void GBASoftComposer::Compose()
    {
        u16 backdrop_color = *m_BdColor;

        // Clear rendering buffer
        memset(m_OutputBuffer, 0, 240 * 160 * sizeof(u16));
        m_FirstLayer = true;

        // Put everything layer-wise together.
        for (int i = 3; i >= 0; i--)
        {
            for (int j = 3; j >= 0; j--)
            {
                if (m_BG[j]->enable && m_BG[j]->priority == i)
                {
                    bool window_inside[3] = { m_Win[0]->bg_in[j], m_Win[1]->bg_in[j], false };
                    DrawLayer(m_BgFinalBuffer[j], window_inside, m_WinOut->bg[j]);
                }
            }

            if (m_Obj->enable)
            {
                bool window_inside[3] = { m_Win[0]->obj_in, m_Win[1]->obj_in, false };
                DrawLayer(m_ObjFinalBuffer[i], window_inside, m_WinOut->obj);
            }
        }

        // Fill any free spots with the BD color.
        for (int k = 0; k < 240 * 160; k++)
        {
            if (m_OutputBuffer[k] == 0x8000)
                m_OutputBuffer[k] = backdrop_color;
        }
    }

    u16* GBASoftComposer::GetOutputBuffer()
    {
        return m_OutputBuffer;
    }
}

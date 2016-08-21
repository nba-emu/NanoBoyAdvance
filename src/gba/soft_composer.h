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

#ifndef __NBA_SOFT_COMPOSER_H__
#define __NBA_SOFT_COMPOSER_H__

#include "composer.h"

namespace NanoboyAdvance
{
    class GBASoftComposer : public GBAComposer
    {
    private:
        inline bool IsVisible(int i, bool inside[3], bool outside)
        {
            if (m_Win[0]->enable || m_Win[1]->enable || m_ObjWin->enable)
            {
                if (m_Win[0]->enable && m_WinFinalMask[0][i] == 1)
                    return inside[0];
                else if (m_Win[1]->enable && m_WinFinalMask[1][i] == 1)
                    return inside[1];
                else
                    return outside;
            }

            return true;
        }

        void ApplySFX(u16* target1, u16 target2);
    public:
        void Update();
        void Compose();
        u16* GetOutputBuffer();
        u16 m_OutputBuffer[240 * 160];
    private:
        u16 m_BgFinalBuffer[4][240 * 160];
        u16 m_ObjFinalBuffer[4][240 * 160];
        u8 m_WinFinalMask[2][240 * 160];
    };
}

#endif // __NBA_SOFT_COMPOSER_H__

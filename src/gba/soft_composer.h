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
    class GBASoftComposer : GBAComposer
    {
    private:
        inline u32 DecodeRGB555(u16 color)
        {
            return 0xFF000000 |
                   (((color & 0x1F) * 8) << 16) |
                   ((((color >> 5) & 0x1F) * 8) << 8) |
                   (((color >> 10) & 0x1F) * 8);
        }
    public:
        void Update();
        void Compose();
    private:
        u32 m_OutputBuffer[240 * 160];
        u32 m_BgFinalBuffer[4][240 * 160];
        u32 m_ObjFinalBuffer[4][240 * 160];
    };
}

#endif // __NBA_SOFT_COMPOSER_H__

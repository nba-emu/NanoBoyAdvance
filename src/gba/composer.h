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

#ifndef __NBA_COMPOSER_H__
#define __NBA_COMPOSER_H__

#include "util/types.h"
#include "video.h"

namespace NanoboyAdvance
{
    class GBAComposer
    {
    public:
        inline void SetVerticalCounter(u16* vcount)
        {
            m_VCount = vcount;
        }

        inline void SetBackgroundBuffer(int id, u16* buffer)
        {
            m_BgBuffer[id] = buffer;
        }

        inline void SetObjectBuffer(int priority, u16* buffer)
        {
            m_ObjBuffer[priority] = buffer;
        }

        inline void SetWindowMaskBuffer(int id, u8* buffer)
        {
            m_WinMask[id] = buffer;
        }

        inline void SetObjectWindowMaskBuffer(u8* buffer)
        {
            m_ObjWinMask = buffer;
        }

        inline void SetBackgroundInfo(int id, struct Background* background)
        {
            m_BG[id] = background;
        }

        inline void SetObjectInfo(struct Object* object)
        {
            m_Obj = object;
        }

        inline void SetWindowInfo(int id, struct Window* window)
        {
            m_Win[id] = window;
        }

        inline void SetObjectWindowInfo(struct ObjectWindow* object_window)
        {
            m_ObjWin = object_window;
        }

        inline void SetWindowOuterInfo(struct WindowOuter* window_outer)
        {
            m_WinOut = window_outer;
        }

        //void SetAlphaBlendBuffer(...);
        //void SetBrightnessBuffer(...);

        virtual void Update() {}
        virtual void Compose() {}

    protected:
        u16* m_VCount;
        u16* m_BgBuffer[4];
        u16* m_ObjBuffer[4];
        u8* m_WinMask[2];
        u8* m_ObjWinMask;
        struct Background* m_BG[4];
        struct Object* m_Obj;
        struct Window* m_Win[2];
        struct ObjectWindow* m_ObjWin;
        struct WindowOuter* m_WinOut;
    };
}

#endif // __NBA_COMPOSER_H__

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
#include "video_extra.h"

namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \file    composer.h
    /// \author  Frederic Meyer
    /// \date    August 6th, 2016
    /// \class   GBAComposer
    /// \brief   Provides a basic parent class for composers to inherit from.
    ///
    ///////////////////////////////////////////////////////////
    class GBAComposer
    {
    public:
        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 6th, 2016
        /// \fn      SetVerticalCounter
        /// \brief   Sets the reference to the vertical counter (aka. current line)
        ///
        /// \param  vcount  Pointer to the vertical counter.
        ///
        ///////////////////////////////////////////////////////////
        inline void SetVerticalCounter(u16* vcount)
        {
            m_VCount = vcount;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 8th, 2016
        /// \fn      SetBackdropColor
        /// \brief   Sets the reference to the backdrop color.
        ///
        /// \param  color  Pointer to the backdrop color.
        ///
        ///////////////////////////////////////////////////////////
        inline void SetBackdropColor(u16* color)
        {
            m_BdColor = color;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 6th, 2016
        /// \fn      SetBackgroundBuffer
        /// \brief   Sets the input buffer for a given background.
        ///
        /// \param  id      Background ID
        /// \param  buffer  Pointer to the RGB555 pixel data.
        ///
        ///////////////////////////////////////////////////////////
        inline void SetBackgroundBuffer(int id, u16* buffer)
        {
            m_BgBuffer[id] = buffer;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 6th, 2016
        /// \fn      SetObjectBuffer
        /// \brief   Sets the input buffer of a object (OAM) layer.
        ///
        /// \param  priority  Layer Priority/ID.
        /// \param  buffer    Pointer to the RGB555 pixel data.
        ///
        ///////////////////////////////////////////////////////////
        inline void SetObjectBuffer(int priority, u16* buffer)
        {
            m_ObjBuffer[priority] = buffer;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 6th, 2016
        /// \fn      SetWindowMaskBuffer
        /// \brief   Sets the mask buffer of one of the windows.
        ///
        /// \param  id     Window ID
        /// \param  buffer Input mask buffer.
        ///
        ///////////////////////////////////////////////////////////
        inline void SetWindowMaskBuffer(int id, u8* buffer)
        {
            m_WinMask[id] = buffer;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 6th, 2016
        /// \fn      SetObjectWindowMaskBuffer
        /// \brief   Sets the object window mask buffer.
        ///
        /// \param   buffer  Input mask buffer.
        ///
        ///////////////////////////////////////////////////////////
        inline void SetObjectWindowMaskBuffer(u8* buffer)
        {
            m_ObjWinMask = buffer;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 7th, 2016
        /// \fn      SetBackgroundInfo
        /// \brief   Sets the information to a background.
        ///
        /// \param  id          Background ID
        /// \param  background  Background Information
        ///
        ///////////////////////////////////////////////////////////
        inline void SetBackgroundInfo(int id, struct Background* background)
        {
            m_BG[id] = background;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 7th, 2016
        /// \fn      SetObjectInfo
        /// \brief   Sets object (OAM) information.
        ///
        /// \param  object  Object Information
        ///
        ///////////////////////////////////////////////////////////
        inline void SetObjectInfo(struct Object* object)
        {
            m_Obj = object;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 7th, 2016
        /// \fn      SetWindowInfo
        /// \brief   Sets the information to a window.
        ///
        /// \param  window  Window Information
        ///
        ///////////////////////////////////////////////////////////
        inline void SetWindowInfo(int id, struct Window* window)
        {
            m_Win[id] = window;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 7th, 2016
        /// \fn      SetObjectWindowInfo
        /// \brief   Sets object window information.
        ///
        /// \param  object_window  Object Window Information
        ///
        ///////////////////////////////////////////////////////////
        inline void SetObjectWindowInfo(struct ObjectWindow* object_window)
        {
            m_ObjWin = object_window;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 7th, 2016
        /// \fn      SetWindowOuterInfo
        /// \brief   Sets information to the outer window area.
        ///
        /// \param  window_outer  Outer Window Information
        ///
        ///////////////////////////////////////////////////////////
        inline void SetWindowOuterInfo(struct WindowOuter* window_outer)
        {
            m_WinOut = window_outer;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 20th, 2016
        /// \fn      SetSFXInfo
        /// \brief   Sets information for Special Effects (SFX).
        ///
        /// \param  sfx  Special Effect Information
        ///
        ///////////////////////////////////////////////////////////
        inline void SetSFXInfo(struct SpecialEffect* sfx)
        {
            m_SFX = sfx;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 6th, 2016
        /// \fn      Scanline
        /// \brief   Composes the final scanline.
        ///
        ///////////////////////////////////////////////////////////
        virtual void Scanline() {}

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 22th, 2016
        /// \fn      Draw
        /// \brief   Draws the finished frame.
        ///
        ///////////////////////////////////////////////////////////
        virtual void Frame() {}

    protected:
        ///////////////////////////////////////////////////////////
        // Class members
        //
        ///////////////////////////////////////////////////////////
        u16* m_VCount;
        u16* m_BdColor; //<<< TODO: big endian support
        u16* m_BgBuffer[4];
        u16* m_ObjBuffer[4];
        u8* m_WinMask[2];
        u8* m_ObjWinMask;
        Background* m_BG[4];
        Object* m_Obj;
        Window* m_Win[2];
        ObjectWindow* m_ObjWin;
        WindowOuter* m_WinOut;
        SpecialEffect* m_SFX;
    };
}

#endif // __NBA_COMPOSER_H__

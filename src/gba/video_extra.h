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

#ifndef __NBA_VIDEO_EXTRA_H__
#define __NBA_VIDEO_EXTRA_H__


#include "util/types.h"


const u16 COLOR_TRANSPARENT = 0x8000;


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
    u32 x_ref {0};
    u32 y_ref {0};
    float x_ref_int {0};
    float y_ref_int {0};
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

///////////////////////////////////////////////////////////
/// \author  Frederic Meyer
/// \date    August 20th, 2016
/// \struct  SpecialEffects
///
/// Describes the state of special effect registers.
///
///////////////////////////////////////////////////////////
struct SpecialEffect
{
    bool bg_select[2][4] {{false, false, false, false},
                          {false, false, false, false}};
    bool obj_select[2] {false, false};
    bool bd_select[2] {false, false};

    enum Effect
    {
        SFX_NONE = 0,
        SFX_ALPHABLEND = 1,
        SFX_INCREASE = 2,
        SFX_DECREASE = 3
    } effect {SFX_NONE};

    int eva {0};
    int evb {0};
    int evy {0};
};

#endif // __NBA_VIDEO_EXTRA_H__

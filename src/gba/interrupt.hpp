///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2017 Frederic Meyer
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

#pragma once

#include "util/integer.hpp"

namespace gba
{
    enum interrupt_type
    {
        INTERRUPT_VBLANK  = 1,
        INTERRUPT_HBLANK  = 2,
        INTERRUPT_VCOUNT  = 4,
        INTERRUPT_TIMER_0 = 8,
        INTERRUPT_TIMER_1 = 16,
        INTERRUPT_TIMER_2 = 32,
        INTERRUPT_TIMER_3 = 64,
        INTERRUPT_SERIAL  = 128,
        INTERRUPT_DMA_0   = 256,
        INTERRUPT_DMA_1   = 512,
        INTERRUPT_DMA_2   = 1024,
        INTERRUPT_DMA_3   = 2048,
        INTERRUPT_KEYPAD  = 4096,
        INTERRUPT_GAMEPAK = 8192
    };

    class interrupt
    {
    private:
        u16* interrupt_flag;

    public:
        void request(interrupt_type type);
        void set_flag_register(u16* io_reg);
    };
}

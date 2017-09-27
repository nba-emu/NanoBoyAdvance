/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

namespace Core {
    enum InterruptType {
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

    enum SystemState {
        SYSTEM_RUN,
        SYSTEM_STOP,
        SYSTEM_HALT
    };

    enum class Key {
        None   = 0,
        A      = 1<<0,
        B      = 1<<1,
        Select = 1<<2,
        Start  = 1<<3,
        Right  = 1<<4,
        Left   = 1<<5,
        Up     = 1<<6,
        Down   = 1<<7,
        R      = 1<<8,
        L      = 1<<9
    };
}

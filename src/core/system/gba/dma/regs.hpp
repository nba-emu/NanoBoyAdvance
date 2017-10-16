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

#include "util/integer.hpp"

namespace Core {

    enum DMAControl {
        DMA_INCREMENT = 0,
        DMA_DECREMENT = 1,
        DMA_FIXED     = 2,
        DMA_RELOAD    = 3
    };

    enum DMATime {
        DMA_IMMEDIATE = 0,
        DMA_VBLANK    = 1,
        DMA_HBLANK    = 2,
        DMA_SPECIAL   = 3
    };

    enum DMASize {
        DMA_HWORD = 0,
        DMA_WORD  = 1
    };

    struct DMA {
        bool enable;
        bool repeat;
        bool interrupt;
        bool gamepak;

        u16 length;
        u32 dst_addr;
        u32 src_addr;
        DMAControl dst_cntl;
        DMAControl src_cntl;
        DMATime time;
        DMASize size;

        struct Internal {
            u32 length;
            u32 dst_addr;
            u32 src_addr;
        } internal;
    };

}

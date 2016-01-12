/*
* Copyright (C) 2015 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <iostream>
#include "common/types.h"
#include "common/log.h"
#include "memory.h"
#include "gba_io.h"

using namespace std;

namespace NanoboyAdvance
{
    class GBADMA
    {
        enum AddressControl
        {
            Increment = 0,
            Decrement = 1,
            Fixed = 2,
            IncrementAndReload = 3
        };
        Memory* memory;
        GBAIO* gba_io;
        void DMA0();
        void DMA1();
        void DMA2();
        void DMA3();
    public:
        u32 dma0_source;
        u32 dma1_source;
        u32 dma2_source;
        u32 dma3_source;
        u32 dma0_destination;
        u32 dma1_destination;
        u32 dma2_destination;
        u32 dma3_destination;
        u16 dma0_count;
        u16 dma1_count;
        u16 dma2_count;
        u16 dma3_count;
        GBADMA(Memory* memory, GBAIO* gba_io);
        void Step();
    };
}

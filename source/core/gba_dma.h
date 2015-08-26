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
#include "../common/types.h"
#include "memory.h"
#include "gba_io.h"

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
        bool irq;
        uint dma0_source { 0 };
        uint dma1_source { 0 };
        uint dma2_source { 0 };
        uint dma3_source { 0 };
        uint dma0_destination { 0 };
        uint dma1_destination { 0 };
        uint dma2_destination { 0 };
        uint dma3_destination { 0 };
        ushort dma0_count { 0 };
        ushort dma1_count { 0 };
        ushort dma2_count { 0 };
        ushort dma3_count { 0 };
        GBADMA(Memory* memory, GBAIO* gba_io);
        void Step();
    };
}

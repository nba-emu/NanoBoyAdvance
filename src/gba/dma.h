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


#ifndef __NBA_DMA_H__
#define __NBA_DMA_H__


namespace GBA
{
    enum AddressControl
    {
        DMA_INCREMENT   = 0,
        DMA_DECREMENT   = 1,
        DMA_FIXED       = 2,
        DMA_RELOAD      = 3
    };

    enum StartTime
    {
        DMA_IMMEDIATE   = 0,
        DMA_VBLANK      = 1,
        DMA_HBLANK      = 2,
        DMA_SPECIAL     = 3
    };

    enum TransferSize
    {
        DMA_HWORD       = 0,
        DMA_WORD        = 1
    };

    struct DMA
    {
        u32 dest;
        u32 source;
        u16 count;
        u32 dest_int;
        u32 source_int;
        u16 count_int;
        AddressControl dest_control;
        AddressControl source_control;
        StartTime      start_time;
        TransferSize   size;
        bool repeat;
        bool gamepack_drq;
        bool interrupt;
        bool enable;
    };
}

#endif // __NBA_DMA_H__

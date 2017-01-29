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

#include "cpu.hpp"

namespace gba
{
    void cpu::io::dma::reset()
    {
        repeat = false;
        gamepak = false;
        interrupt = false;
        enable = false;
        length = 0;
        dst_addr = 0;
        src_addr = 0;
        dst_cntl = DMA_INCREMENT;
        src_cntl = DMA_INCREMENT;
        time = DMA_IMMEDIATE;
        size = DMA_HWORD;
        internal.length = 0;
        internal.dst_addr = 0;
        internal.src_addr = 0;
    }

    auto cpu::io::dma::read(int offset) -> u8
    {
        switch (offset)
        {
        }
    }

    void cpu::io::dma::write(int offset, u8 value)
    {
    }

    void cpu::io::timer::reset()
    {
        ticks = 0;
        reload = 0;
        counter = 0;
        control.frequency = 0;
        control.cascade = false;
        control.interrupt = false;
        control.enable = false;
    }

    auto cpu::io::timer::read(int offset) -> u8
    {
        switch (offset)
        {
        case 0: return counter & 0xFF;
        case 1: return counter >> 8;
        case 2:
            return (control.frequency) |
                   (control.cascade ? 4 : 0) |
                   (control.interrupt ? 64 : 0) |
                   (control.enable ? 128 : 0);
        }
    }

    void cpu::io::timer::write(int offset, u8 value)
    {
        switch (offset)
        {
        case 0: reload = (reload & 0xFF00) | value; break;
        case 1: reload = (reload & 0x00FF) | (value << 8); break;
        case 2:
        {
            bool enable_previous = control.enable;

            control.frequency = value & 3;
            control.cascade = value & 4;
            control.interrupt = value & 64;
            control.enable = value & 128;

            if (!enable_previous && control.enable)
            {
                counter = reload;
            }
        }
        }
    }

    void cpu::io::interrupt_io::reset()
    {
        enable = 0;
        request = 0;
        master_enable = 0;
    }
}

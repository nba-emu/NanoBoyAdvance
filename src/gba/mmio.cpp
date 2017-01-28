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
#include "util/logger.hpp"

using namespace util;

namespace gba
{
    u8 cpu::read_mmio(u32 address)
    {
        //logger::log<LOG_INFO>("IO read from 0x{0:x} r15={1:x}", address, m_reg[15]);

        switch (address)
        {
        // DISPCNT
        case 0x04000000:
            return m_ppu.get_io().control.read(0);
        case 0x04000001:
            return m_ppu.get_io().control.read(1);

        // DISPSTAT
        case 0x04000004:
            return m_ppu.get_io().status.read(0);
        case 0x04000005:
            return m_ppu.get_io().status.read(1);

        // VCOUNT
        case 0x04000006:
            return m_ppu.get_io().vcount & 0xFF;
        case 0x04000007:
            return m_ppu.get_io().vcount >> 8;

        // BG0CNT
        case 0x04000008:
            return m_ppu.get_io().bgcnt[0].read(0);
        case 0x04000009:
            return m_ppu.get_io().bgcnt[0].read(1);
        // BG1CNT
        case 0x0400000A:
            return m_ppu.get_io().bgcnt[1].read(0);
        case 0x0400000B:
            return m_ppu.get_io().bgcnt[1].read(1);
        // BG2CNT
        case 0x0400000C:
            return m_ppu.get_io().bgcnt[2].read(0);
        case 0x0400000D:
            return m_ppu.get_io().bgcnt[2].read(1);
        // BG3CNT
        case 0x0400000E:
            return m_ppu.get_io().bgcnt[2].read(0);
        case 0x0400000F:
            return m_ppu.get_io().bgcnt[2].read(1);

        // KEYINPUT
        case 0x04000130:
            return m_io.keyinput & 0xFF;
        case 0x04000131:
            return m_io.keyinput >> 8;

        // IE
        case 0x04000200:
            return m_io.interrupt.enable & 0xFF;
        case 0x04000201:
            return m_io.interrupt.enable >> 8;

        // IF
        case 0x04000202:
            return m_io.interrupt.request & 0xFF;
        case 0x04000203:
            return m_io.interrupt.request >> 8;

        // IME
        case 0x04000208:
            return m_io.interrupt.master_enable & 0xFF;
        case 0x04000209:
            return m_io.interrupt.master_enable >> 8;
        }

        return 0;
    }

    void cpu::write_mmio(u32 address, u8 value)
    {
        //logger::log<LOG_INFO>("IO write to 0x{0:x}={1:x} r15={2:x}", address, value, m_reg[15]);

        switch (address)
        {
        // DISPCNT
        case 0x04000000:
            m_ppu.get_io().control.write(0, value);
            break;
        case 0x04000001:
            m_ppu.get_io().control.write(1, value);
            break;

        // DISPSTAT
        case 0x04000004:
            m_ppu.get_io().status.write(0, value);
            break;
        case 0x04000005:
            m_ppu.get_io().status.write(1, value);
            break;

        // BG0CNT
        case 0x04000008:
            m_ppu.get_io().bgcnt[0].write(0, value);
            break;
        case 0x04000009:
            m_ppu.get_io().bgcnt[0].write(1, value);
            break;
        // BG1CNT
        case 0x0400000A:
            m_ppu.get_io().bgcnt[1].write(0, value);
            break;
        case 0x0400000B:
            m_ppu.get_io().bgcnt[1].write(1, value);
            break;
        // BG2CNT
        case 0x0400000C:
            m_ppu.get_io().bgcnt[2].write(0, value);
            break;
        case 0x0400000D:
            m_ppu.get_io().bgcnt[2].write(1, value);
            break;
        // BG3CNT
        case 0x0400000E:
            m_ppu.get_io().bgcnt[3].write(0, value);
            break;
        case 0x0400000F:
            m_ppu.get_io().bgcnt[3].write(1, value);
            break;

        // IE
        case 0x04000200:
            m_io.interrupt.enable &= 0xFF00;
            m_io.interrupt.enable |= value;
            break;
        case 0x04000201:
            m_io.interrupt.enable &= 0x00FF;
            m_io.interrupt.enable |= (value << 8);
            break;

        // IF
        case 0x04000202:
            m_io.interrupt.request &= ~value;
            break;
        case 0x04000203:
            m_io.interrupt.request &= ~(value << 8);
            break;

        // IME
        case 0x04000208:
            m_io.interrupt.master_enable &= 0xFF00;
            m_io.interrupt.master_enable |= value;
            break;
        case 0x04000209:
            m_io.interrupt.master_enable &= 0x00FF;
            m_io.interrupt.master_enable |= (value << 8);
            break;
        }
    }
}

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

// TODO: cleanup and simplify. Use inliners to reduce code redundancy.

#include "cpu.hpp"
#include "mmio.hpp"
#include "util/logger.hpp"

using namespace util;

namespace gba
{
    u8 cpu::read_mmio(u32 address)
    {
        auto& ppu_io = m_ppu.get_io();

        switch (address)
        {
        // PPU
        case DISPCNT:    return ppu_io.control.read(0);
        case DISPCNT+1:  return ppu_io.control.read(1);
        case DISPSTAT:   return ppu_io.status.read(0);
        case DISPSTAT+1: return ppu_io.status.read(1);
        case VCOUNT:     return ppu_io.vcount & 0xFF;
        case VCOUNT+1:   return ppu_io.vcount >> 8;
        case BG0CNT:     return ppu_io.bgcnt[0].read(0);
        case BG0CNT+1:   return ppu_io.bgcnt[0].read(1);
        case BG1CNT:     return ppu_io.bgcnt[1].read(0);
        case BG1CNT+1:   return ppu_io.bgcnt[1].read(1);
        case BG2CNT:     return ppu_io.bgcnt[2].read(0);
        case BG2CNT+1:   return ppu_io.bgcnt[2].read(1);
        case BG3CNT:     return ppu_io.bgcnt[2].read(0);
        case BG3CNT+1:   return ppu_io.bgcnt[2].read(1);

        // JOYPAD
        case KEYINPUT:   return m_io.keyinput & 0xFF;
        case KEYINPUT+1: return m_io.keyinput >> 8;

        // INTERRUPT
        case IE:    return m_io.interrupt.enable & 0xFF;
        case IE+1:  return m_io.interrupt.enable >> 8;
        case IF:    return m_io.interrupt.request & 0xFF;
        case IF+1:  return m_io.interrupt.request >> 8;
        case IME:   return m_io.interrupt.master_enable & 0xFF;
        case IME+1: return m_io.interrupt.master_enable >> 8;
        }

        return 0;
    }

    void cpu::write_mmio(u32 address, u8 value)
    {
        auto& ppu_io = m_ppu.get_io();

        switch (address)
        {
        // PPU
        case DISPCNT:    ppu_io.control.write(0, value); break;
        case DISPCNT+1:  ppu_io.control.write(1, value); break;
        case DISPSTAT:   ppu_io.status.write(0, value); break;
        case DISPSTAT+1: ppu_io.status.write(1, value); break;
        case BG0CNT:     ppu_io.bgcnt[0].write(0, value); break;
        case BG0CNT+1:   ppu_io.bgcnt[0].write(1, value); break;
        case BG1CNT:     ppu_io.bgcnt[1].write(0, value); break;
        case BG1CNT+1:   ppu_io.bgcnt[1].write(1, value); break;
        case BG2CNT:     ppu_io.bgcnt[2].write(0, value); break;
        case BG2CNT+1:   ppu_io.bgcnt[2].write(1, value); break;
        case BG3CNT:     ppu_io.bgcnt[3].write(0, value); break;
        case BG3CNT+1:   ppu_io.bgcnt[3].write(1, value); break;

        case BG0HOFS:
            ppu_io.bghofs[0] = (ppu_io.bghofs[0] & 0xFF00) | value;
            break;
        case BG0HOFS+1:
            ppu_io.bghofs[0] = (ppu_io.bghofs[0] & 0x00FF) | (value << 8);
            break;
        case BG0VOFS:
            ppu_io.bgvofs[0] = (ppu_io.bgvofs[0] & 0xFF00) | value;
            break;
        case BG0VOFS+1:
            ppu_io.bgvofs[0] = (ppu_io.bgvofs[0] & 0x00FF) | (value << 8);
            break;
        case BG1HOFS:
            ppu_io.bghofs[1] = (ppu_io.bghofs[1] & 0xFF00) | value;
            break;
        case BG1HOFS+1:
            ppu_io.bghofs[1] = (ppu_io.bghofs[1] & 0x00FF) | (value << 8);
            break;
        case BG1VOFS:
            ppu_io.bgvofs[1] = (ppu_io.bgvofs[1] & 0xFF00) | value;
            break;
        case BG1VOFS+1:
            ppu_io.bgvofs[1] = (ppu_io.bgvofs[1] & 0x00FF) | (value << 8);
            break;
        case BG2HOFS:
            ppu_io.bghofs[2] = (ppu_io.bghofs[2] & 0xFF00) | value;
            break;
        case BG2HOFS+1:
            ppu_io.bghofs[2] = (ppu_io.bghofs[2] & 0x00FF) | (value << 8);
            break;
        case BG2VOFS:
            ppu_io.bgvofs[2] = (ppu_io.bgvofs[2] & 0xFF00) | value;
            break;
        case BG2VOFS+1:
            ppu_io.bgvofs[2] = (ppu_io.bgvofs[2] & 0x00FF) | (value << 8);
            break;
        case BG3HOFS:
            ppu_io.bghofs[3] = (ppu_io.bghofs[3] & 0xFF00) | value;
            break;
        case BG3HOFS+1:
            ppu_io.bghofs[3] = (ppu_io.bghofs[3] & 0x00FF) | (value << 8);
            break;
        case BG3VOFS:
            ppu_io.bgvofs[3] = (ppu_io.bgvofs[3] & 0xFF00) | value;
            break;
        case BG3VOFS+1:
            ppu_io.bgvofs[3] = (ppu_io.bgvofs[3] & 0x00FF) | (value << 8);
            break;

        // INTERRUPT
        case IE:
            m_io.interrupt.enable &= 0xFF00;
            m_io.interrupt.enable |= value;
            break;
        case IE+1:
            m_io.interrupt.enable &= 0x00FF;
            m_io.interrupt.enable |= (value << 8);
            break;
        case IF:
            m_io.interrupt.request &= ~value;
            break;
        case IF+1:
            m_io.interrupt.request &= ~(value << 8);
            break;
        case IME:
            m_io.interrupt.master_enable &= 0xFF00;
            m_io.interrupt.master_enable |= value;
            break;
        case IME+1:
            m_io.interrupt.master_enable &= 0x00FF;
            m_io.interrupt.master_enable |= (value << 8);
            break;
        }
    }
}

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

// TODO: cleanup and simplify. Use inliners to reduce code redundancy.

#include "cpu.hpp"
#include "mmio.hpp"
#include "util/logger.hpp"

using namespace util;

namespace GameBoyAdvance {
    
    u8 CPU::read_mmio(u32 address) {
        auto& ppu_io = m_ppu.get_io();

        logger::log<LOG_INFO>("io read address={0:x} pc={1:x} vcount={2}", address, m_reg[15], ppu_io.vcount);

        switch (address) {
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

            // DMA
            case DMA0CNT_H:   return m_io.dma[0].read(10);
            case DMA0CNT_H+1: return m_io.dma[0].read(11);
            case DMA1CNT_H:   return m_io.dma[1].read(10);
            case DMA1CNT_H+1: return m_io.dma[1].read(11);
            case DMA2CNT_H:   return m_io.dma[2].read(10);
            case DMA2CNT_H+1: return m_io.dma[2].read(11);
            case DMA3CNT_H:   return m_io.dma[3].read(10);
            case DMA3CNT_H+1: return m_io.dma[3].read(11);

            // TIMER
            case TM0CNT_L:   return m_io.timer[0].read(0);
            case TM0CNT_L+1: return m_io.timer[0].read(1);
            case TM0CNT_H:   return m_io.timer[0].read(2);
            case TM1CNT_L:   return m_io.timer[1].read(0);
            case TM1CNT_L+1: return m_io.timer[1].read(1);
            case TM1CNT_H:   return m_io.timer[1].read(2);
            case TM2CNT_L:   return m_io.timer[2].read(0);
            case TM2CNT_L+1: return m_io.timer[2].read(1);
            case TM2CNT_H:   return m_io.timer[2].read(2);
            case TM3CNT_L:   return m_io.timer[3].read(0);
            case TM3CNT_L+1: return m_io.timer[3].read(1);
            case TM3CNT_H:   return m_io.timer[3].read(2);

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

            default: logger::log<LOG_DEBUG>("unknown IO read {0:X}", address);
        }

        return m_mmio[address & 0x7FF];
    }

    void CPU::write_mmio(u32 address, u8 value) {
        auto& ppu_io = m_ppu.get_io();

        //logger::log<LOG_INFO>("io write address={0:x} value={1:x}", address, value);

        switch (address) {
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

            case BG2PA:   ppu_io.bgpa[0] = (ppu_io.bgpa[0] & 0xFF00) | (value << 0); break;
            case BG2PA+1: ppu_io.bgpa[0] = (ppu_io.bgpa[0] & 0x00FF) | (value << 8); break;
            case BG2PB:   ppu_io.bgpb[0] = (ppu_io.bgpb[0] & 0xFF00) | (value << 0); break;
            case BG2PB+1: ppu_io.bgpb[0] = (ppu_io.bgpb[0] & 0x00FF) | (value << 8); break;
            case BG2PC:   ppu_io.bgpc[0] = (ppu_io.bgpc[0] & 0xFF00) | (value << 0); break;
            case BG2PC+1: ppu_io.bgpc[0] = (ppu_io.bgpc[0] & 0x00FF) | (value << 8); break;
            case BG2PD:   ppu_io.bgpd[0] = (ppu_io.bgpd[0] & 0xFF00) | (value << 0); break;
            case BG2PD+1: ppu_io.bgpd[0] = (ppu_io.bgpd[0] & 0x00FF) | (value << 8); break;
            case BG2X:    ppu_io.bgx[0].write(0, value); break;
            case BG2X+1:  ppu_io.bgx[0].write(1, value); break;
            case BG2X+2:  ppu_io.bgx[0].write(2, value); break;
            case BG2X+3:  ppu_io.bgx[0].write(3, value); break;
            case BG2Y:    ppu_io.bgy[0].write(0, value); break;
            case BG2Y+1:  ppu_io.bgy[0].write(1, value); break;
            case BG2Y+2:  ppu_io.bgy[0].write(2, value); break;
            case BG2Y+3:  ppu_io.bgy[0].write(3, value); break;
            case BG3PA:   ppu_io.bgpa[1] = (ppu_io.bgpa[1] & 0xFF00) | (value << 0); break;
            case BG3PA+1: ppu_io.bgpa[1] = (ppu_io.bgpa[1] & 0x00FF) | (value << 8); break;
            case BG3PB:   ppu_io.bgpb[1] = (ppu_io.bgpb[1] & 0xFF00) | (value << 0); break;
            case BG3PB+1: ppu_io.bgpb[1] = (ppu_io.bgpb[1] & 0x00FF) | (value << 8); break;
            case BG3PC:   ppu_io.bgpc[1] = (ppu_io.bgpc[1] & 0xFF00) | (value << 0); break;
            case BG3PC+1: ppu_io.bgpc[1] = (ppu_io.bgpc[1] & 0x00FF) | (value << 8); break;
            case BG3PD:   ppu_io.bgpd[1] = (ppu_io.bgpd[1] & 0xFF00) | (value << 0); break;
            case BG3PD+1: ppu_io.bgpd[1] = (ppu_io.bgpd[1] & 0x00FF) | (value << 8); break;
            case BG3X:    ppu_io.bgx[1].write(0, value); break;
            case BG3X+1:  ppu_io.bgx[1].write(1, value); break;
            case BG3X+2:  ppu_io.bgx[1].write(2, value); break;
            case BG3X+3:  ppu_io.bgx[1].write(3, value); break;
            case BG3Y:    ppu_io.bgy[1].write(0, value); break;
            case BG3Y+1:  ppu_io.bgy[1].write(1, value); break;
            case BG3Y+2:  ppu_io.bgy[1].write(2, value); break;
            case BG3Y+3:  ppu_io.bgy[1].write(3, value); break;

            // DMA
            case DMA0SAD:     m_io.dma[0].write(0, value); break;
            case DMA0SAD+1:   m_io.dma[0].write(1, value); break;
            case DMA0SAD+2:   m_io.dma[0].write(2, value); break;
            case DMA0SAD+3:   m_io.dma[0].write(3, value); break;
            case DMA0DAD:     m_io.dma[0].write(4, value); break;
            case DMA0DAD+1:   m_io.dma[0].write(5, value); break;
            case DMA0DAD+2:   m_io.dma[0].write(6, value); break;
            case DMA0DAD+3:   m_io.dma[0].write(7, value); break;
            case DMA0CNT_L:   m_io.dma[0].write(8, value); break;
            case DMA0CNT_L+1: m_io.dma[0].write(9, value); break;
            case DMA0CNT_H:   m_io.dma[0].write(10, value); break;
            case DMA0CNT_H+1: m_io.dma[0].write(11, value); break;
            case DMA1SAD:     m_io.dma[1].write(0, value); break;
            case DMA1SAD+1:   m_io.dma[1].write(1, value); break;
            case DMA1SAD+2:   m_io.dma[1].write(2, value); break;
            case DMA1SAD+3:   m_io.dma[1].write(3, value); break;
            case DMA1DAD:     m_io.dma[1].write(4, value); break;
            case DMA1DAD+1:   m_io.dma[1].write(5, value); break;
            case DMA1DAD+2:   m_io.dma[1].write(6, value); break;
            case DMA1DAD+3:   m_io.dma[1].write(7, value); break;
            case DMA1CNT_L:   m_io.dma[1].write(8, value); break;
            case DMA1CNT_L+1: m_io.dma[1].write(9, value); break;
            case DMA1CNT_H:   m_io.dma[1].write(10, value); break;
            case DMA1CNT_H+1: m_io.dma[1].write(11, value); break;
            case DMA2SAD:     m_io.dma[2].write(0, value); break;
            case DMA2SAD+1:   m_io.dma[2].write(1, value); break;
            case DMA2SAD+2:   m_io.dma[2].write(2, value); break;
            case DMA2SAD+3:   m_io.dma[2].write(3, value); break;
            case DMA2DAD:     m_io.dma[2].write(4, value); break;
            case DMA2DAD+1:   m_io.dma[2].write(5, value); break;
            case DMA2DAD+2:   m_io.dma[2].write(6, value); break;
            case DMA2DAD+3:   m_io.dma[2].write(7, value); break;
            case DMA2CNT_L:   m_io.dma[2].write(8, value); break;
            case DMA2CNT_L+1: m_io.dma[2].write(9, value); break;
            case DMA2CNT_H:   m_io.dma[2].write(10, value); break;
            case DMA2CNT_H+1: m_io.dma[2].write(11, value); break;
            case DMA3SAD:     m_io.dma[3].write(0, value); break;
            case DMA3SAD+1:   m_io.dma[3].write(1, value); break;
            case DMA3SAD+2:   m_io.dma[3].write(2, value); break;
            case DMA3SAD+3:   m_io.dma[3].write(3, value); break;
            case DMA3DAD:     m_io.dma[3].write(4, value); break;
            case DMA3DAD+1:   m_io.dma[3].write(5, value); break;
            case DMA3DAD+2:   m_io.dma[3].write(6, value); break;
            case DMA3DAD+3:   m_io.dma[3].write(7, value); break;
            case DMA3CNT_L:   m_io.dma[3].write(8, value); break;
            case DMA3CNT_L+1: m_io.dma[3].write(9, value); break;
            case DMA3CNT_H:   m_io.dma[3].write(10, value); break;
            case DMA3CNT_H+1: m_io.dma[3].write(11, value); break;

            // TIMER
            case TM0CNT_L:   m_io.timer[0].write(0, value); break;
            case TM0CNT_L+1: m_io.timer[0].write(1, value); break;
            case TM0CNT_H:   m_io.timer[0].write(2, value); break;
            case TM1CNT_L:   m_io.timer[1].write(0, value); break;
            case TM1CNT_L+1: m_io.timer[1].write(1, value); break;
            case TM1CNT_H:   m_io.timer[1].write(2, value); break;
            case TM2CNT_L:   m_io.timer[2].write(0, value); break;
            case TM2CNT_L+1: m_io.timer[2].write(1, value); break;
            case TM2CNT_H:   m_io.timer[2].write(2, value); break;
            case TM3CNT_L:   m_io.timer[3].write(0, value); break;
            case TM3CNT_L+1: m_io.timer[3].write(1, value); break;
            case TM3CNT_H:   m_io.timer[3].write(2, value); break;

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
            case HALTCNT:
                m_io.haltcnt = (value & 0x80) ? SYSTEM_STOP : SYSTEM_HALT;
                break;

            default:
                logger::log<LOG_DEBUG>("unknown IO write {0:X}", address);
                m_mmio[address & 0x7FF] = value;
        }
    }
}

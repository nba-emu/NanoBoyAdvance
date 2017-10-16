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

#include "mmio.hpp"
#include "../emulator.hpp"
#include "util/logger.hpp"

using namespace Util;

namespace Core {

    auto Emulator::readMMIO(u32 address) -> u8 {
        auto& ppu_io = ppu.getIO();
        auto& apu_io = apu.getIO();

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
            case WININ:      return ppu_io.winin.read(0);
            case WININ+1:    return ppu_io.winin.read(1);
            case WINOUT:     return ppu_io.winout.read(0);
            case WINOUT+1:   return ppu_io.winout.read(1);
            case BLDCNT:     return ppu_io.bldcnt.read(0);
            case BLDCNT+1:   return ppu_io.bldcnt.read(1);

            // DMA01
            case DMA0CNT_H:   return dmaRead(0, 10);
            case DMA0CNT_H+1: return dmaRead(0, 11);
            case DMA1CNT_H:   return dmaRead(1, 10);
            case DMA1CNT_H+1: return dmaRead(1, 11);
            case DMA2CNT_H:   return dmaRead(2, 10);
            case DMA2CNT_H+1: return dmaRead(2, 11);
            case DMA3CNT_H:   return dmaRead(3, 10);
            case DMA3CNT_H+1: return dmaRead(3, 11);

            // SOUND
            case SOUND1CNT_L:   return apu_io.tone[0].read(0);
            case SOUND1CNT_L+1: return apu_io.tone[0].read(1);
            case SOUND1CNT_H:   return apu_io.tone[0].read(2);
            case SOUND1CNT_H+1: return apu_io.tone[0].read(3);
            case SOUND1CNT_X:   return apu_io.tone[0].read(4);
            case SOUND1CNT_X+1: return apu_io.tone[0].read(5);
            case SOUND2CNT_L:   return apu_io.tone[1].read(2);
            case SOUND2CNT_L+1: return apu_io.tone[1].read(3);
            case SOUND2CNT_H:   return apu_io.tone[1].read(4);
            case SOUND2CNT_H+1: return apu_io.tone[1].read(5);
            case SOUND3CNT_L:   return apu_io.wave.read(0);
            case SOUND3CNT_L+1: return apu_io.wave.read(1);
            case SOUND3CNT_H:   return apu_io.wave.read(2);
            case SOUND3CNT_H+1: return apu_io.wave.read(3);
            case SOUND3CNT_X:   return apu_io.wave.read(4);
            case SOUND3CNT_X+1: return apu_io.wave.read(5);
            case SOUND4CNT_L:   return apu_io.noise.read(0);
            case SOUND4CNT_L+1: return apu_io.noise.read(1);
            case SOUND4CNT_H:   return apu_io.noise.read(4);
            case SOUND4CNT_H+1: return apu_io.noise.read(5);
            case SOUNDCNT_L:    return apu_io.control.read(0);
            case SOUNDCNT_L+1:  return apu_io.control.read(1);
            case SOUNDCNT_H:    return apu_io.control.read(2);
            case SOUNDCNT_H+1:  return apu_io.control.read(3);
            case SOUNDCNT_X:    return apu_io.control.read(4);

            // TIMER
            case TM0CNT_L:   return timerRead(0, 0);
            case TM0CNT_L+1: return timerRead(0, 1);
            case TM0CNT_H:   return timerRead(0, 2);
            case TM1CNT_L:   return timerRead(1, 0);
            case TM1CNT_L+1: return timerRead(1, 1);
            case TM1CNT_H:   return timerRead(1, 2);
            case TM2CNT_L:   return timerRead(2, 0);
            case TM2CNT_L+1: return timerRead(2, 1);
            case TM2CNT_H:   return timerRead(2, 2);
            case TM3CNT_L:   return timerRead(3, 0);
            case TM3CNT_L+1: return timerRead(3, 1);
            case TM3CNT_H:   return timerRead(3, 2);

            // JOYPAD
            case KEYINPUT:   return regs.keyinput & 0xFF;
            case KEYINPUT+1: return regs.keyinput >> 8;

            // INTERRUPT
            case IE:    return regs.irq.enable & 0xFF;
            case IE+1:  return regs.irq.enable >> 8;
            case IF:    return regs.irq.flag & 0xFF;
            case IF+1:  return regs.irq.flag >> 8;
            case IME:   return regs.irq.master_enable & 0xFF;
            case IME+1: return regs.irq.master_enable >> 8;

            // WAITSTATES
            case WAITCNT: {
                const auto& waitcnt = regs.waitcnt;

                return (waitcnt.sram  << 0) |
                       (waitcnt.ws0_n << 2) |
                       (waitcnt.ws0_s << 4) |
                       (waitcnt.ws1_n << 5) |
                       (waitcnt.ws1_s << 7) ;
            }
            case WAITCNT+1: {
                const auto& waitcnt = regs.waitcnt;

                return (waitcnt.ws2_n    << 0) |
                       (waitcnt.ws2_s    << 2) |
                       (waitcnt.phi      << 3) |
                       (waitcnt.prefetch << 6) |
                       (waitcnt.cgb      << 7) ;
            }

            default: Logger::log<LOG_DEBUG>("unknown IO read {0:X}", address);
        }

        return memory.mmio[address & 0x7FF];
    }

    void Emulator::writeMMIO(u32 address, u8 value) {
        auto& ppu_io = ppu.getIO();
        auto& apu_io = apu.getIO();

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

            case BG0HOFS:   ppu_io.bghofs[0] = (ppu_io.bghofs[0] & 0xFF00) | value;        break;
            case BG0HOFS+1: ppu_io.bghofs[0] = (ppu_io.bghofs[0] & 0x00FF) | (value << 8); break;
            case BG0VOFS:   ppu_io.bgvofs[0] = (ppu_io.bgvofs[0] & 0xFF00) | value;        break;
            case BG0VOFS+1: ppu_io.bgvofs[0] = (ppu_io.bgvofs[0] & 0x00FF) | (value << 8); break;
            case BG1HOFS:   ppu_io.bghofs[1] = (ppu_io.bghofs[1] & 0xFF00) | value;        break;
            case BG1HOFS+1: ppu_io.bghofs[1] = (ppu_io.bghofs[1] & 0x00FF) | (value << 8); break;
            case BG1VOFS:   ppu_io.bgvofs[1] = (ppu_io.bgvofs[1] & 0xFF00) | value;        break;
            case BG1VOFS+1: ppu_io.bgvofs[1] = (ppu_io.bgvofs[1] & 0x00FF) | (value << 8); break;
            case BG2HOFS:   ppu_io.bghofs[2] = (ppu_io.bghofs[2] & 0xFF00) | value;        break;
            case BG2HOFS+1: ppu_io.bghofs[2] = (ppu_io.bghofs[2] & 0x00FF) | (value << 8); break;
            case BG2VOFS:   ppu_io.bgvofs[2] = (ppu_io.bgvofs[2] & 0xFF00) | value;        break;
            case BG2VOFS+1: ppu_io.bgvofs[2] = (ppu_io.bgvofs[2] & 0x00FF) | (value << 8); break;
            case BG3HOFS:   ppu_io.bghofs[3] = (ppu_io.bghofs[3] & 0xFF00) | value;        break;
            case BG3HOFS+1: ppu_io.bghofs[3] = (ppu_io.bghofs[3] & 0x00FF) | (value << 8); break;
            case BG3VOFS:   ppu_io.bgvofs[3] = (ppu_io.bgvofs[3] & 0xFF00) | value;        break;
            case BG3VOFS+1: ppu_io.bgvofs[3] = (ppu_io.bgvofs[3] & 0x00FF) | (value << 8); break;

            case BG2PA:      ppu_io.bgpa[0] = (ppu_io.bgpa[0] & 0xFF00) | (value << 0); break;
            case BG2PA+1:    ppu_io.bgpa[0] = (ppu_io.bgpa[0] & 0x00FF) | (value << 8); break;
            case BG2PB:      ppu_io.bgpb[0] = (ppu_io.bgpb[0] & 0xFF00) | (value << 0); break;
            case BG2PB+1:    ppu_io.bgpb[0] = (ppu_io.bgpb[0] & 0x00FF) | (value << 8); break;
            case BG2PC:      ppu_io.bgpc[0] = (ppu_io.bgpc[0] & 0xFF00) | (value << 0); break;
            case BG2PC+1:    ppu_io.bgpc[0] = (ppu_io.bgpc[0] & 0x00FF) | (value << 8); break;
            case BG2PD:      ppu_io.bgpd[0] = (ppu_io.bgpd[0] & 0xFF00) | (value << 0); break;
            case BG2PD+1:    ppu_io.bgpd[0] = (ppu_io.bgpd[0] & 0x00FF) | (value << 8); break;
            case BG2X:       ppu_io.bgx[0].write(0, value); break;
            case BG2X+1:     ppu_io.bgx[0].write(1, value); break;
            case BG2X+2:     ppu_io.bgx[0].write(2, value); break;
            case BG2X+3:     ppu_io.bgx[0].write(3, value); break;
            case BG2Y:       ppu_io.bgy[0].write(0, value); break;
            case BG2Y+1:     ppu_io.bgy[0].write(1, value); break;
            case BG2Y+2:     ppu_io.bgy[0].write(2, value); break;
            case BG2Y+3:     ppu_io.bgy[0].write(3, value); break;
            case BG3PA:      ppu_io.bgpa[1] = (ppu_io.bgpa[1] & 0xFF00) | (value << 0); break;
            case BG3PA+1:    ppu_io.bgpa[1] = (ppu_io.bgpa[1] & 0x00FF) | (value << 8); break;
            case BG3PB:      ppu_io.bgpb[1] = (ppu_io.bgpb[1] & 0xFF00) | (value << 0); break;
            case BG3PB+1:    ppu_io.bgpb[1] = (ppu_io.bgpb[1] & 0x00FF) | (value << 8); break;
            case BG3PC:      ppu_io.bgpc[1] = (ppu_io.bgpc[1] & 0xFF00) | (value << 0); break;
            case BG3PC+1:    ppu_io.bgpc[1] = (ppu_io.bgpc[1] & 0x00FF) | (value << 8); break;
            case BG3PD:      ppu_io.bgpd[1] = (ppu_io.bgpd[1] & 0xFF00) | (value << 0); break;
            case BG3PD+1:    ppu_io.bgpd[1] = (ppu_io.bgpd[1] & 0x00FF) | (value << 8); break;
            case BG3X:       ppu_io.bgx[1].write(0, value); break;
            case BG3X+1:     ppu_io.bgx[1].write(1, value); break;
            case BG3X+2:     ppu_io.bgx[1].write(2, value); break;
            case BG3X+3:     ppu_io.bgx[1].write(3, value); break;
            case BG3Y:       ppu_io.bgy[1].write(0, value); break;
            case BG3Y+1:     ppu_io.bgy[1].write(1, value); break;
            case BG3Y+2:     ppu_io.bgy[1].write(2, value); break;
            case BG3Y+3:     ppu_io.bgy[1].write(3, value); break;
            case WIN0H:      ppu_io.winh[0].write(0, value); break;
            case WIN0H+1:    ppu_io.winh[0].write(1, value); break;
            case WIN1H:      ppu_io.winh[1].write(0, value); break;
            case WIN1H+1:    ppu_io.winh[1].write(1, value); break;
            case WIN0V:      ppu_io.winv[0].write(0, value); break;
            case WIN0V+1:    ppu_io.winv[0].write(1, value); break;
            case WIN1V:      ppu_io.winv[1].write(0, value); break;
            case WIN1V+1:    ppu_io.winv[1].write(1, value); break;
            case WININ:      ppu_io.winin.write(0, value); break;
            case WININ+1:    ppu_io.winin.write(1, value); break;
            case WINOUT:     ppu_io.winout.write(0, value); break;
            case WINOUT+1:   ppu_io.winout.write(1, value); break;
            case MOSAIC:     ppu_io.mosaic.write(0, value); break;
            case MOSAIC+1:   ppu_io.mosaic.write(1, value); break;
            case BLDCNT:     ppu_io.bldcnt.write(0, value); break;
            case BLDCNT+1:   ppu_io.bldcnt.write(1, value); break;
            case BLDALPHA:   ppu_io.bldalpha.write(0, value); break;
            case BLDALPHA+1: ppu_io.bldalpha.write(1, value); break;
            case BLDY:       ppu_io.bldy.write(value); break;

            // DMA
            case DMA0SAD:     dmaWrite(0, 0, value); break;
            case DMA0SAD+1:   dmaWrite(0, 1, value); break;
            case DMA0SAD+2:   dmaWrite(0, 2, value); break;
            case DMA0SAD+3:   dmaWrite(0, 3, value); break;
            case DMA0DAD:     dmaWrite(0, 4, value); break;
            case DMA0DAD+1:   dmaWrite(0, 5, value); break;
            case DMA0DAD+2:   dmaWrite(0, 6, value); break;
            case DMA0DAD+3:   dmaWrite(0, 7, value); break;
            case DMA0CNT_L:   dmaWrite(0, 8, value); break;
            case DMA0CNT_L+1: dmaWrite(0, 9, value); break;
            case DMA0CNT_H:   dmaWrite(0, 10, value); break;
            case DMA0CNT_H+1: dmaWrite(0, 11, value); break;
            case DMA1SAD:     dmaWrite(1, 0, value); break;
            case DMA1SAD+1:   dmaWrite(1, 1, value); break;
            case DMA1SAD+2:   dmaWrite(1, 2, value); break;
            case DMA1SAD+3:   dmaWrite(1, 3, value); break;
            case DMA1DAD:     dmaWrite(1, 4, value); break;
            case DMA1DAD+1:   dmaWrite(1, 5, value); break;
            case DMA1DAD+2:   dmaWrite(1, 6, value); break;
            case DMA1DAD+3:   dmaWrite(1, 7, value); break;
            case DMA1CNT_L:   dmaWrite(1, 8, value); break;
            case DMA1CNT_L+1: dmaWrite(1, 9, value); break;
            case DMA1CNT_H:   dmaWrite(1, 10, value); break;
            case DMA1CNT_H+1: dmaWrite(1, 11, value); break;
            case DMA2SAD:     dmaWrite(2, 0, value); break;
            case DMA2SAD+1:   dmaWrite(2, 1, value); break;
            case DMA2SAD+2:   dmaWrite(2, 2, value); break;
            case DMA2SAD+3:   dmaWrite(2, 3, value); break;
            case DMA2DAD:     dmaWrite(2, 4, value); break;
            case DMA2DAD+1:   dmaWrite(2, 5, value); break;
            case DMA2DAD+2:   dmaWrite(2, 6, value); break;
            case DMA2DAD+3:   dmaWrite(2, 7, value); break;
            case DMA2CNT_L:   dmaWrite(2, 8, value); break;
            case DMA2CNT_L+1: dmaWrite(2, 9, value); break;
            case DMA2CNT_H:   dmaWrite(2, 10, value); break;
            case DMA2CNT_H+1: dmaWrite(2, 11, value); break;
            case DMA3SAD:     dmaWrite(3, 0, value); break;
            case DMA3SAD+1:   dmaWrite(3, 1, value); break;
            case DMA3SAD+2:   dmaWrite(3, 2, value); break;
            case DMA3SAD+3:   dmaWrite(3, 3, value); break;
            case DMA3DAD:     dmaWrite(3, 4, value); break;
            case DMA3DAD+1:   dmaWrite(3, 5, value); break;
            case DMA3DAD+2:   dmaWrite(3, 6, value); break;
            case DMA3DAD+3:   dmaWrite(3, 7, value); break;
            case DMA3CNT_L:   dmaWrite(3, 8, value); break;
            case DMA3CNT_L+1: dmaWrite(3, 9, value); break;
            case DMA3CNT_H:   dmaWrite(3, 10, value); break;
            case DMA3CNT_H+1: dmaWrite(3, 11, value); break;

            // SOUND
            case SOUND1CNT_L:   apu_io.tone[0].write(0, value); break;
            case SOUND1CNT_L+1: apu_io.tone[0].write(1, value); break;
            case SOUND1CNT_H:   apu_io.tone[0].write(2, value); break;
            case SOUND1CNT_H+1: apu_io.tone[0].write(3, value); break;
            case SOUND1CNT_X:   apu_io.tone[0].write(4, value); break;
            case SOUND1CNT_X+1: apu_io.tone[0].write(5, value); break;
            case SOUND2CNT_L:   apu_io.tone[1].write(2, value); break;
            case SOUND2CNT_L+1: apu_io.tone[1].write(3, value); break;
            case SOUND2CNT_H:   apu_io.tone[1].write(4, value); break;
            case SOUND2CNT_H+1: apu_io.tone[1].write(5, value); break;
            case SOUND3CNT_L:   apu_io.wave.write(0, value); break;
            case SOUND3CNT_L+1: apu_io.wave.write(1, value); break;
            case SOUND3CNT_H:   apu_io.wave.write(2, value); break;
            case SOUND3CNT_H+1: apu_io.wave.write(3, value); break;
            case SOUND3CNT_X:   apu_io.wave.write(4, value); break;
            case SOUND3CNT_X+1: apu_io.wave.write(5, value); break;
            case SOUND4CNT_L:   apu_io.noise.write(0, value); break;
            case SOUND4CNT_L+1: apu_io.noise.write(1, value); break;
            case SOUND4CNT_H:   apu_io.noise.write(4, value); break;
            case SOUND4CNT_H+1: apu_io.noise.write(5, value); break;
            case WAVE_RAM+0:
            case WAVE_RAM+1:
            case WAVE_RAM+2:
            case WAVE_RAM+3:
            case WAVE_RAM+4:
            case WAVE_RAM+5:
            case WAVE_RAM+6:
            case WAVE_RAM+7:
            case WAVE_RAM+8:
            case WAVE_RAM+9:
            case WAVE_RAM+10:
            case WAVE_RAM+11:
            case WAVE_RAM+12:
            case WAVE_RAM+13:
            case WAVE_RAM+14:
            case WAVE_RAM+15: {
                int index = address & 0xF;
                int bank  = apu_io.wave.bank_number ^ 1;
                apu_io.wave_ram[bank][index] = value;
                break;
            }
            case FIFO_A:
            case FIFO_A+1:
            case FIFO_A+2:
            case FIFO_A+3: apu_io.fifo[0].enqueue(value); break;
            case FIFO_B:
            case FIFO_B+1:
            case FIFO_B+2:
            case FIFO_B+3: apu_io.fifo[1].enqueue(value); break;
            case SOUNDCNT_L:   apu_io.control.write(0, value); break;
            case SOUNDCNT_L+1: apu_io.control.write(1, value); break;
            case SOUNDCNT_H:   apu_io.control.write(2, value); break;
            case SOUNDCNT_H+1: apu_io.control.write(3, value); break;
            case SOUNDCNT_X:   apu_io.control.write(4, value); break;

            // TIMER
            case TM0CNT_L:   timerWrite(0, 0, value); break;
            case TM0CNT_L+1: timerWrite(0, 1, value); break;
            case TM0CNT_H:   timerWrite(0, 2, value); break;
            case TM1CNT_L:   timerWrite(1, 0, value); break;
            case TM1CNT_L+1: timerWrite(1, 1, value); break;
            case TM1CNT_H:   timerWrite(1, 2, value); break;
            case TM2CNT_L:   timerWrite(2, 0, value); break;
            case TM2CNT_L+1: timerWrite(2, 1, value); break;
            case TM2CNT_H:   timerWrite(2, 2, value); break;
            case TM3CNT_L:   timerWrite(3, 0, value); break;
            case TM3CNT_L+1: timerWrite(3, 1, value); break;
            case TM3CNT_H:   timerWrite(3, 2, value); break;

            // INTERRUPT
            case IE: {
                regs.irq.enable &= 0xFF00;
                regs.irq.enable |= value;
                break;
            }
            case IE+1: {
                regs.irq.enable &= 0x00FF;
                regs.irq.enable |= (value << 8);
                break;
            }
            case IF: {
                regs.irq.flag &= ~value;
                break;
            }
            case IF+1: {
                regs.irq.flag &= ~(value << 8);
                break;
            }
            case IME: {
                regs.irq.master_enable &= 0xFF00;
                regs.irq.master_enable |= value;
                break;
            }
            case IME+1: {
                regs.irq.master_enable &= 0x00FF;
                regs.irq.master_enable |= (value << 8);
                break;
            }

            // WAITSTATES
            case WAITCNT: {
                auto& waitcnt = regs.waitcnt;
                waitcnt.sram  = (value >> 0) & 3;
                waitcnt.ws0_n = (value >> 2) & 3;
                waitcnt.ws0_s = (value >> 4) & 1;
                waitcnt.ws1_n = (value >> 5) & 3;
                waitcnt.ws1_s = (value >> 7) & 1;
                calculateMemoryCycles();
                break;
            }
            case WAITCNT+1: {
                auto& waitcnt = regs.waitcnt;
                waitcnt.ws2_n    = (value >> 0) & 3;
                waitcnt.ws2_s    = (value >> 2) & 1;
                waitcnt.phi      = (value >> 3) & 3;
                waitcnt.prefetch = (value >> 6) & 1;
                waitcnt.cgb      = (value >> 7) & 1;
                calculateMemoryCycles();
                break;
            }

            // SYSTEM CONTROL
            case HALTCNT: {
                regs.haltcnt = (value & 0x80) ? SYSTEM_STOP : SYSTEM_HALT;
                break;
            }

            default: {
                Logger::log<LOG_DEBUG>("unknown IO write {0:X}", address);
                memory.mmio[address & 0x7FF] = value;
            }
        }
    }
}

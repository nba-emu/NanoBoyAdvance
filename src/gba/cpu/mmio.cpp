/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
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

#include <cstdio>

#include "cpu.hpp"
#include "mmio.hpp"

using namespace GameBoyAdvance;

using Key = InputDevice::Key;

auto CPU::ReadMMIO(std::uint32_t address) -> std::uint8_t {
  auto& apu_io = apu.mmio;
  auto& ppu_io = ppu.mmio;

  switch (address) {
    /* PPU */
    case DISPCNT+0:  return ppu_io.dispcnt.Read(0);
    case DISPCNT+1:  return ppu_io.dispcnt.Read(1);
    case DISPSTAT+0: return ppu_io.dispstat.Read(0);
    case DISPSTAT+1: return ppu_io.dispstat.Read(1);
    case VCOUNT+0:   return ppu_io.vcount & 0xFF;
    case VCOUNT+1:   return 0;
    case BG0CNT+0:   return ppu_io.bgcnt[0].Read(0);
    case BG0CNT+1:   return ppu_io.bgcnt[0].Read(1);
    case BG1CNT+0:   return ppu_io.bgcnt[1].Read(0);
    case BG1CNT+1:   return ppu_io.bgcnt[1].Read(1);
    case BG2CNT+0:   return ppu_io.bgcnt[2].Read(0);
    case BG2CNT+1:   return ppu_io.bgcnt[2].Read(1);
    case BG3CNT+0:   return ppu_io.bgcnt[3].Read(0);
    case BG3CNT+1:   return ppu_io.bgcnt[3].Read(1);
    case WININ+0:    return ppu_io.winin.Read(0);
    case WININ+1:    return ppu_io.winin.Read(1);
    case WINOUT+0:   return ppu_io.winout.Read(0);
    case WINOUT+1:   return ppu_io.winout.Read(1);
    case BLDCNT+0:   return ppu_io.bldcnt.Read(0);
    case BLDCNT+1:   return ppu_io.bldcnt.Read(1);
    
    /* DMAs 0-3 */
    case DMA0CNT_H:   return ReadDMA(0, 10);
    case DMA0CNT_H+1: return ReadDMA(0, 11);
    case DMA1CNT_H:   return ReadDMA(1, 10);
    case DMA1CNT_H+1: return ReadDMA(1, 11);
    case DMA2CNT_H:   return ReadDMA(2, 10);
    case DMA2CNT_H+1: return ReadDMA(2, 11);
    case DMA3CNT_H:   return ReadDMA(3, 10);
    case DMA3CNT_H+1: return ReadDMA(3, 11);
      
    /* SOUND */
    case SOUNDCNT_L:   return apu_io.soundcnt.Read(0);
    case SOUNDCNT_L+1: return apu_io.soundcnt.Read(1);
    case SOUNDCNT_H:   return apu_io.soundcnt.Read(2);
    case SOUNDCNT_H+1: return apu_io.soundcnt.Read(3);
    case SOUNDCNT_X:   return apu_io.soundcnt.Read(4);
    case SOUNDBIAS:    return apu_io.bias.Read(0);
    case SOUNDBIAS+1:  return apu_io.bias.Read(1);
      
    /* Timers 0-3 */
    case TM0CNT_L:   return timers.Read(0, 0);
    case TM0CNT_L+1: return timers.Read(0, 1);
    case TM0CNT_H:   return timers.Read(0, 2);
    case TM1CNT_L:   return timers.Read(1, 0);
    case TM1CNT_L+1: return timers.Read(1, 1);
    case TM1CNT_H:   return timers.Read(1, 2);
    case TM2CNT_L:   return timers.Read(2, 0);
    case TM2CNT_L+1: return timers.Read(2, 1);
    case TM2CNT_H:   return timers.Read(2, 2);
    case TM3CNT_L:   return timers.Read(3, 0);
    case TM3CNT_L+1: return timers.Read(3, 1);
    case TM3CNT_H:   return timers.Read(3, 2);
  
    case KEYINPUT+0: {
      return (config->input_dev->Poll(Key::A)      ? 0 : 1)  |
             (config->input_dev->Poll(Key::B)      ? 0 : 2)  |
             (config->input_dev->Poll(Key::Select) ? 0 : 4)  |
             (config->input_dev->Poll(Key::Start)  ? 0 : 8)  |
             (config->input_dev->Poll(Key::Right)  ? 0 : 16) |
             (config->input_dev->Poll(Key::Left)   ? 0 : 32) |
             (config->input_dev->Poll(Key::Up)     ? 0 : 64) |
             (config->input_dev->Poll(Key::Down)   ? 0 : 128);
    }
    case KEYINPUT+1: {
      return (config->input_dev->Poll(Key::R) ? 0 : 1) |
             (config->input_dev->Poll(Key::L) ? 0 : 2);
    }
      
    /* Interrupt Control */
    case IE+0:  return mmio.irq_ie  & 0xFF;
    case IE+1:  return mmio.irq_ie  >> 8;
    case IF+0:  return mmio.irq_if  & 0xFF;
    case IF+1:  return mmio.irq_if  >> 8;
    case IME+0: return mmio.irq_ime & 0xFF;
    case IME+1: return mmio.irq_ime >> 8;
  }
  return 0;
}

void CPU::WriteMMIO(std::uint32_t address, std::uint8_t value) {
  auto& apu_io = apu.mmio;
  auto& ppu_io = ppu.mmio;

  switch (address) {
    /* PPU */
    case DISPCNT+0:  ppu_io.dispcnt.Write(0, value); break;
    case DISPCNT+1:  ppu_io.dispcnt.Write(1, value); break;
    case DISPSTAT+0: ppu_io.dispstat.Write(0, value); break;
    case DISPSTAT+1: ppu_io.dispstat.Write(1, value); break;
    case BG0CNT+0:   ppu_io.bgcnt[0].Write(0, value); break;
    case BG0CNT+1:   ppu_io.bgcnt[0].Write(1, value); break;
    case BG1CNT+0:   ppu_io.bgcnt[1].Write(0, value); break;
    case BG1CNT+1:   ppu_io.bgcnt[1].Write(1, value); break;
    case BG2CNT+0:   ppu_io.bgcnt[2].Write(0, value); break;
    case BG2CNT+1:   ppu_io.bgcnt[2].Write(1, value); break;
    case BG3CNT+0:   ppu_io.bgcnt[3].Write(0, value); break;
    case BG3CNT+1:   ppu_io.bgcnt[3].Write(1, value); break;
    case BG0HOFS+0:
      ppu_io.bghofs[0] &= 0xFF00;
      ppu_io.bghofs[0] |= value;
      break;
    case BG0HOFS+1:
      ppu_io.bghofs[0] &= 0x00FF;
      ppu_io.bghofs[0] |= (value & 1) << 8;
      break;
    case BG0VOFS+0:
      ppu_io.bgvofs[0] &= 0xFF00;
      ppu_io.bgvofs[0] |= value;
      break;
    case BG0VOFS+1:
      ppu_io.bgvofs[0] &= 0x00FF;
      ppu_io.bgvofs[0] |= (value & 1) << 8;
      break;
    case BG1HOFS+0:
      ppu_io.bghofs[1] &= 0xFF00;
      ppu_io.bghofs[1] |= value;
      break;
    case BG1HOFS+1:
      ppu_io.bghofs[1] &= 0x00FF;
      ppu_io.bghofs[1] |= (value & 1) << 8;
      break;
    case BG1VOFS+0:
      ppu_io.bgvofs[1] &= 0xFF00;
      ppu_io.bgvofs[1] |= value;
      break;
    case BG1VOFS+1:
      ppu_io.bgvofs[1] &= 0x00FF;
      ppu_io.bgvofs[1] |= (value & 1) << 8;
      break;
    case BG2HOFS+0:
      ppu_io.bghofs[2] &= 0xFF00;
      ppu_io.bghofs[2] |= value;
      break;
    case BG2HOFS+1:
      ppu_io.bghofs[2] &= 0x00FF;
      ppu_io.bghofs[2] |= (value & 1) << 8;
      break;
    case BG2VOFS+0:
      ppu_io.bgvofs[2] &= 0xFF00;
      ppu_io.bgvofs[2] |= value;
      break;
    case BG2VOFS+1:
      ppu_io.bgvofs[2] &= 0x00FF;
      ppu_io.bgvofs[2] |= (value & 1) << 8;
      break;
    case BG3HOFS+0:
      ppu_io.bghofs[3] &= 0xFF00;
      ppu_io.bghofs[3] |= value;
      break;
    case BG3HOFS+1:
      ppu_io.bghofs[3] &= 0x00FF;
      ppu_io.bghofs[3] |= (value & 1) << 8;
      break;
    case BG3VOFS+0:
      ppu_io.bgvofs[3] &= 0xFF00;
      ppu_io.bgvofs[3] |= value;
      break;
    case BG3VOFS+1:
      ppu_io.bgvofs[3] &= 0x00FF;
      ppu_io.bgvofs[3] |= (value & 1) << 8;
      break;
    case WIN0H+0: ppu_io.winh[0].Write(0, value); break;
    case WIN0H+1: ppu_io.winh[0].Write(1, value); break;
    case WIN1H+0: ppu_io.winh[1].Write(0, value); break;
    case WIN1H+1: ppu_io.winh[1].Write(1, value); break;    
    case WIN0V+0: ppu_io.winv[0].Write(0, value); break;
    case WIN0V+1: ppu_io.winv[0].Write(1, value); break;
    case WIN1V+0: ppu_io.winv[1].Write(0, value); break;
    case WIN1V+1: ppu_io.winv[1].Write(1, value); break;
    case WININ+0: ppu_io.winin.Write(0, value); break;
    case WININ+1: ppu_io.winin.Write(1, value); break;
    case WINOUT+0: ppu_io.winout.Write(0, value); break;
    case WINOUT+1: ppu_io.winout.Write(1, value); break;
    case BLDCNT+0: ppu_io.bldcnt.Write(0, value); break;
    case BLDCNT+1: ppu_io.bldcnt.Write(1, value); break;
    case BLDALPHA+0:
      ppu_io.eva = value & 0x1F;
      break;
    case BLDALPHA+1:
      ppu_io.evb = value & 0x1F;
      break;
    case BLDY:
      ppu_io.evy = value & 0x1F;
      break;
    
    /* DMAs 0-3 */
    case DMA0SAD:     WriteDMA(0, 0, value); break;
    case DMA0SAD+1:   WriteDMA(0, 1, value); break;
    case DMA0SAD+2:   WriteDMA(0, 2, value); break;
    case DMA0SAD+3:   WriteDMA(0, 3, value); break;
    case DMA0DAD:     WriteDMA(0, 4, value); break;
    case DMA0DAD+1:   WriteDMA(0, 5, value); break;
    case DMA0DAD+2:   WriteDMA(0, 6, value); break;
    case DMA0DAD+3:   WriteDMA(0, 7, value); break;
    case DMA0CNT_L:   WriteDMA(0, 8, value); break;
    case DMA0CNT_L+1: WriteDMA(0, 9, value); break;
    case DMA0CNT_H:   WriteDMA(0, 10, value); break;
    case DMA0CNT_H+1: WriteDMA(0, 11, value); break;
    case DMA1SAD:     WriteDMA(1, 0, value); break;
    case DMA1SAD+1:   WriteDMA(1, 1, value); break;
    case DMA1SAD+2:   WriteDMA(1, 2, value); break;
    case DMA1SAD+3:   WriteDMA(1, 3, value); break;
    case DMA1DAD:     WriteDMA(1, 4, value); break;
    case DMA1DAD+1:   WriteDMA(1, 5, value); break;
    case DMA1DAD+2:   WriteDMA(1, 6, value); break;
    case DMA1DAD+3:   WriteDMA(1, 7, value); break;
    case DMA1CNT_L:   WriteDMA(1, 8, value); break;
    case DMA1CNT_L+1: WriteDMA(1, 9, value); break;
    case DMA1CNT_H:   WriteDMA(1, 10, value); break;
    case DMA1CNT_H+1: WriteDMA(1, 11, value); break;
    case DMA2SAD:     WriteDMA(2, 0, value); break;
    case DMA2SAD+1:   WriteDMA(2, 1, value); break;
    case DMA2SAD+2:   WriteDMA(2, 2, value); break;
    case DMA2SAD+3:   WriteDMA(2, 3, value); break;
    case DMA2DAD:     WriteDMA(2, 4, value); break;
    case DMA2DAD+1:   WriteDMA(2, 5, value); break;
    case DMA2DAD+2:   WriteDMA(2, 6, value); break;
    case DMA2DAD+3:   WriteDMA(2, 7, value); break;
    case DMA2CNT_L:   WriteDMA(2, 8, value); break;
    case DMA2CNT_L+1: WriteDMA(2, 9, value); break;
    case DMA2CNT_H:   WriteDMA(2, 10, value); break;
    case DMA2CNT_H+1: WriteDMA(2, 11, value); break;
    case DMA3SAD:     WriteDMA(3, 0, value); break;
    case DMA3SAD+1:   WriteDMA(3, 1, value); break;
    case DMA3SAD+2:   WriteDMA(3, 2, value); break;
    case DMA3SAD+3:   WriteDMA(3, 3, value); break;
    case DMA3DAD:     WriteDMA(3, 4, value); break;
    case DMA3DAD+1:   WriteDMA(3, 5, value); break;
    case DMA3DAD+2:   WriteDMA(3, 6, value); break;
    case DMA3DAD+3:   WriteDMA(3, 7, value); break;
    case DMA3CNT_L:   WriteDMA(3, 8, value); break;
    case DMA3CNT_L+1: WriteDMA(3, 9, value); break;
    case DMA3CNT_H:   WriteDMA(3, 10, value); break;
    case DMA3CNT_H+1: WriteDMA(3, 11, value); break;
      
    /* SOUND */
    case FIFO_A:
    case FIFO_A+1:
    case FIFO_A+2:
    case FIFO_A+3: apu_io.fifo[0].Write(value); break;
    case FIFO_B:
    case FIFO_B+1:
    case FIFO_B+2:
    case FIFO_B+3: apu_io.fifo[1].Write(value); break;
    case SOUNDCNT_L:   apu_io.soundcnt.Write(0, value); break;
    case SOUNDCNT_L+1: apu_io.soundcnt.Write(1, value); break;
    case SOUNDCNT_H:   apu_io.soundcnt.Write(2, value); break;
    case SOUNDCNT_H+1: apu_io.soundcnt.Write(3, value); break;
    case SOUNDCNT_X:   apu_io.soundcnt.Write(4, value); break;
    case SOUNDBIAS:    apu_io.bias.Write(0, value);
    case SOUNDBIAS+1:  apu_io.bias.Write(1, value);
      
    /* Timers 0-3 */
    case TM0CNT_L:   timers.Write(0, 0, value); break;
    case TM0CNT_L+1: timers.Write(0, 1, value); break;
    case TM0CNT_H:   timers.Write(0, 2, value); break;
    case TM1CNT_L:   timers.Write(1, 0, value); break;
    case TM1CNT_L+1: timers.Write(1, 1, value); break;
    case TM1CNT_H:   timers.Write(1, 2, value); break;
    case TM2CNT_L:   timers.Write(2, 0, value); break;
    case TM2CNT_L+1: timers.Write(2, 1, value); break;
    case TM2CNT_H:   timers.Write(2, 2, value); break;
    case TM3CNT_L:   timers.Write(3, 0, value); break;
    case TM3CNT_L+1: timers.Write(3, 1, value); break;
    case TM3CNT_H:   timers.Write(3, 2, value); break;
    
    /* Interrupt Control */
    case IE+0: {
      mmio.irq_ie &= 0xFF00;
      mmio.irq_ie |= value;
      break;
    }
    case IE+1: {
      mmio.irq_ie &= 0x00FF;
      mmio.irq_ie |= value << 8;
      break;
    }
    case IF+0: {
      mmio.irq_if &= ~value;
      break;
    }
    case IF+1: {
      mmio.irq_if &= ~(value << 8);
      break;
    }
    case IME+0: {
      mmio.irq_ime &= 0xFF00;
      mmio.irq_ime |= value;
      break;
    }
    case IME+1: {
      mmio.irq_ime &= 0x00FF;
      mmio.irq_ime |= value << 8;
      break;
    }

    /* Waitstates */
    case WAITCNT+0: {
      mmio.waitcnt.sram  = (value >> 0) & 3;
      mmio.waitcnt.ws0_n = (value >> 2) & 3;
      mmio.waitcnt.ws0_s = (value >> 4) & 1;
      mmio.waitcnt.ws1_n = (value >> 5) & 3;
      mmio.waitcnt.ws1_s = (value >> 7) & 1;
      UpdateCycleLUT();
      break;
    }
    case WAITCNT+1: {
      mmio.waitcnt.ws2_n = (value >> 0) & 3;
      mmio.waitcnt.ws2_s = (value >> 2) & 1;
      mmio.waitcnt.phi = (value >> 3) & 3;
      mmio.waitcnt.prefetch = (value >> 6) & 1;
      mmio.waitcnt.cgb = (value >> 7) & 1;
      UpdateCycleLUT();
      break;
    }

    case HALTCNT: {
      if (value & 0x80) {
        mmio.haltcnt = HaltControl::STOP;
      } else {
        mmio.haltcnt = HaltControl::HALT;
      }
      break;
    }
  }
}

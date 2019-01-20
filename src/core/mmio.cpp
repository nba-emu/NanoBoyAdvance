/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstdio>

#include "cpu.hpp"
#include "mmio.hpp"

using namespace NanoboyAdvance::GBA;

auto CPU::ReadMMIO(std::uint32_t address) -> std::uint8_t {
    //std::printf("[R][MMIO] 0x%08x\n", address);

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
        case BLDCNT+0:   return ppu_io.bldcnt.Read(0);
        case BLDCNT+1:   return ppu_io.bldcnt.Read(1);
        
        case KEYINPUT+0: return mmio.keyinput & 0xFF;
        case KEYINPUT+1: return mmio.keyinput >> 8;

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
    //std::printf("[W][MMIO] 0x%08x=0x%02x\n", address, value);

    auto& ppu_io = ppu.mmio;

    switch (address) {
        /* PPU */
        case DISPCNT+0:  ppu_io.dispcnt.Write(0, value); break;
        case DISPCNT+1:  ppu_io.dispcnt.Write(1, value); break;
        case DISPSTAT+0: ppu_io.dispstat.Write(0, value); break;
        case DISPSTAT+1: ppu_io.dispstat.Write(1, value); break;
        /* TODO: do VCOUNT writes have an effect on the GBA too? */
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

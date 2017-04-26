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

#include "cpu.hpp"
#include "util/logger.hpp"

using namespace Util;

namespace GameBoyAdvance {
    
    void CPU::IO::DMA::reset() {
        enable = false;
        repeat = false;
        interrupt = false;
        gamepak   = false;

        length = 0;
        dst_addr = 0;
        src_addr = 0;
        internal.length = 0;
        internal.dst_addr = 0;
        internal.src_addr = 0;

        size = DMA_HWORD;
        time = DMA_IMMEDIATE;
        dst_cntl = DMA_INCREMENT;
        src_cntl = DMA_INCREMENT;
    }

    auto CPU::IO::DMA::read(int offset) -> u8 {
        // TODO: are SAD/DAD/CNT_L readable?
        switch (offset) {
            // DMAXCNT_H
            case 10:
                return (dst_cntl << 5) |
                       (src_cntl << 7);
            case 11:
                return (src_cntl >> 1) |
                       (size     << 2) |
                       (time     << 4) |
                       (repeat    ? 2   : 0) |
                       (gamepak   ? 8   : 0) |
                       (interrupt ? 64  : 0) |
                       (enable    ? 128 : 0);
        }
    }

    void CPU::IO::DMA::write(int offset, u8 value) {
        switch (offset) {
            // DMAXSAD
            case 0: src_addr = (src_addr & 0xFFFFFF00) | value; break;
            case 1: src_addr = (src_addr & 0xFFFF00FF) | (value<<8); break;
            case 2: src_addr = (src_addr & 0xFF00FFFF) | (value<<16); break;
            case 3: src_addr = (src_addr & 0x00FFFFFF) | (value<<24); break;

            // DMAXDAD
            case 4: dst_addr = (dst_addr & 0xFFFFFF00) | value; break;
            case 5: dst_addr = (dst_addr & 0xFFFF00FF) | (value<<8); break;
            case 6: dst_addr = (dst_addr & 0xFF00FFFF) | (value<<16); break;
            case 7: dst_addr = (dst_addr & 0x00FFFFFF) | (value<<24); break;

            // DMAXCNT_L
            case 8: length = (length & 0xFF00) | value; break;
            case 9: length = (length & 0x00FF) | (value<<8); break;

            // DMAXCNT_H
            case 10:
                dst_cntl = static_cast<DMAControl>((value >> 5) & 3);
                src_cntl = static_cast<DMAControl>((src_cntl & 0b10) | (value>>7));
                break;
            case 11: {
                bool enable_previous = enable;

                src_cntl  = static_cast<DMAControl>((src_cntl & 0b01) | ((value & 1)<<1));
                size      = static_cast<DMASize>((value>>2) & 1);
                time      = static_cast<DMATime>((value>>4) & 3);
                repeat    = value & 2;
                gamepak   = value & 8;
                interrupt = value & 64;
                enable    = value & 128;

                if (!enable_previous && enable) {
                    // TODO(accuracy): length, src/dst_addr must be masked.
                    internal.length = length;
                    internal.dst_addr = dst_addr;
                    internal.src_addr = src_addr;

                    if (time == DMA_IMMEDIATE) {
                        // !!hacked!! sad flerovium :(
                        *dma_active = true;
                        *current_dma = id;
                    }
                }
                break;
            }
        }
    }

    void CPU::IO::Timer::reset() {
        cycles = 0;
        reload = 0;
        counter = 0;
        control.frequency = 0;
        control.cascade = false;
        control.interrupt = false;
        control.enable = false;
    }

    auto CPU::IO::Timer::read(int offset) -> u8 {
        switch (offset) {
            case 0: return counter & 0xFF;
            case 1: return counter >> 8;
            case 2:
                return (control.frequency) |
                       (control.cascade   ? 4   : 0) |
                       (control.interrupt ? 64  : 0) |
                       (control.enable    ? 128 : 0);
        }
    }

    void CPU::IO::Timer::write(int offset, u8 value) {
        switch (offset) {
            case 0: reload = (reload & 0xFF00) | value; break;
            case 1: reload = (reload & 0x00FF) | (value << 8); break;
            case 2: {
                bool enable_previous = control.enable;

                control.frequency = value & 3;
                control.cascade   = value & 4;
                control.interrupt = value & 64;
                control.enable    = value & 128;

                if (!enable_previous && control.enable) {
                    counter = reload;
                }
            }
        }
    }

    void CPU::IO::Interrupt::reset() {
        enable = 0;
        request = 0;
        master_enable = 0;
    }
}

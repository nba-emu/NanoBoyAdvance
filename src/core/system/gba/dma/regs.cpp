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

#include "../emulator.hpp"

#include "util/logger.hpp"

using namespace Util;

namespace Core {
    void Emulator::dmaReset(int id) {
        auto& dma = regs.dma[id];

        dma.enable    = false;
        dma.repeat    = false;
        dma.interrupt = false;
        dma.gamepak   = false;

        dma.length   = 0;
        dma.dst_addr = 0;
        dma.src_addr = 0;
        dma.internal.length   = 0;
        dma.internal.dst_addr = 0;
        dma.internal.src_addr = 0;

        dma.size     = DMA_HWORD;
        dma.time     = DMA_IMMEDIATE;
        dma.dst_cntl = DMA_INCREMENT;
        dma.src_cntl = DMA_INCREMENT;
    }

    auto Emulator::dmaRead(int id, int offset) -> u8 {
        auto& dma = regs.dma[id];

        // TODO: are SAD/DAD/CNT_L readable?
        switch (offset) {
            // DMAXCNT_H
            case 10: {
                return (dma.dst_cntl << 5) |
                       (dma.src_cntl << 7);
            }
            case 11: {
                return (dma.src_cntl >> 1) |
                       (dma.size     << 2) |
                       (dma.time     << 4) |
                       (dma.repeat    ? 2   : 0) |
                       (dma.gamepak   ? 8   : 0) |
                       (dma.interrupt ? 64  : 0) |
                       (dma.enable    ? 128 : 0);
            }
            default: return 0;
        }
    }

    void Emulator::dmaWrite(int id, int offset, u8 value) {
        auto& dma = regs.dma[id];

        switch (offset) {
            // DMAXSAD
            case 0: dma.src_addr = (dma.src_addr & 0xFFFFFF00) | (value<<0 ); break;
            case 1: dma.src_addr = (dma.src_addr & 0xFFFF00FF) | (value<<8 ); break;
            case 2: dma.src_addr = (dma.src_addr & 0xFF00FFFF) | (value<<16); break;
            case 3: dma.src_addr = (dma.src_addr & 0x00FFFFFF) | (value<<24); break;

            // DMAXDAD
            case 4: dma.dst_addr = (dma.dst_addr & 0xFFFFFF00) | (value<<0 ); break;
            case 5: dma.dst_addr = (dma.dst_addr & 0xFFFF00FF) | (value<<8 ); break;
            case 6: dma.dst_addr = (dma.dst_addr & 0xFF00FFFF) | (value<<16); break;
            case 7: dma.dst_addr = (dma.dst_addr & 0x00FFFFFF) | (value<<24); break;

            // DMAXCNT_L
            case 8: dma.length = (dma.length & 0xFF00) | (value<<0); break;
            case 9: dma.length = (dma.length & 0x00FF) | (value<<8); break;

            // DMAXCNT_H
            case 10:
                dma.dst_cntl = static_cast<DMAControl>((value >> 5) & 3);
                dma.src_cntl = static_cast<DMAControl>((dma.src_cntl & 0b10) | (value>>7));
                break;
            case 11: {
                bool enable_previous = dma.enable;

                dma.src_cntl  = static_cast<DMAControl>((dma.src_cntl & 0b01) | ((value & 1)<<1));
                dma.size      = static_cast<DMASize>((value>>2) & 1);
                dma.time      = static_cast<DMATime>((value>>4) & 3);
                dma.repeat    = value & 2;
                dma.gamepak   = value & 8;
                dma.interrupt = value & 64;
                dma.enable    = value & 128;

                // Detect "rising edge" of enable bit.
                if (!enable_previous && dma.enable) {
                    // Load masked values into internal DMA state.
                    dma.internal.dst_addr = dma.dst_addr & s_dma_dst_mask[id];
                    dma.internal.src_addr = dma.src_addr & s_dma_src_mask[id];
                    dma.internal.length   = dma.length   & s_dma_len_mask[id];

                    if (dma.internal.length == 0) dma.internal.length = s_dma_len_mask[id] + 1;

                    // Directly schedule immediate DMAs for execution.
                    if (dma.time == DMA_IMMEDIATE) dmaActivate(id);

                    Logger::log<LOG_DEBUG>("DMA[{0}]: ENABLE: src={1:x}, dst={2:x}", id, dma.internal.src_addr, dma.internal.dst_addr);
                }
                break;
            }
        }
    }
}

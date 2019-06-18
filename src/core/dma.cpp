/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "cpu.hpp"

using namespace NanoboyAdvance::GBA;


void CPU::ResetDMAs() {
    for (int id = 0; id < 4; id++) {
        auto& dma = mmio.dma[id];

        dma.enable = false;
        dma.repeat = false;
        dma.interrupt = false;
        dma.gamepak  = false;
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
}

auto CPU::ReadDMA(int id, int offset) -> std::uint8_t {
    auto& dma = mmio.dma[id];

    /* TODO: are SAD/DAD/CNT_L readable? */
    switch (offset) {
        /* DMAXCNT_H */
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

void CPU::WriteDMA(int id, int offset, std::uint8_t value) {
    auto& dma = mmio.dma[id];

    switch (offset) {
        /* DMAXSAD */
        case 0: dma.src_addr = (dma.src_addr & 0xFFFFFF00) | (value<<0 ); break;
        case 1: dma.src_addr = (dma.src_addr & 0xFFFF00FF) | (value<<8 ); break;
        case 2: dma.src_addr = (dma.src_addr & 0xFF00FFFF) | (value<<16); break;
        case 3: dma.src_addr = (dma.src_addr & 0x00FFFFFF) | (value<<24); break;

        /* DMAXDAD */
        case 4: dma.dst_addr = (dma.dst_addr & 0xFFFFFF00) | (value<<0 ); break;
        case 5: dma.dst_addr = (dma.dst_addr & 0xFFFF00FF) | (value<<8 ); break;
        case 6: dma.dst_addr = (dma.dst_addr & 0xFF00FFFF) | (value<<16); break;
        case 7: dma.dst_addr = (dma.dst_addr & 0x00FFFFFF) | (value<<24); break;

        /* DMAXCNT_L */
        case 8: dma.length = (dma.length & 0xFF00) | (value<<0); break;
        case 9: dma.length = (dma.length & 0x00FF) | (value<<8); break;

        /* DMAXCNT_H */
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

            /* DMA state is latched on "rising" enable bit. */
            if (!enable_previous && dma.enable) {
                /* Latch sanitized values into internal DMA state. */
                dma.internal.dst_addr = dma.dst_addr & s_dma_dst_mask[id];
                dma.internal.src_addr = dma.src_addr & s_dma_src_mask[id];
                dma.internal.length   = dma.length   & s_dma_len_mask[id];

                if (dma.internal.length == 0) {
                    dma.internal.length = s_dma_len_mask[id] + 1;
                }

                /* Schedule DMA if is setup for immediate execution. */
                if (dma.time == DMA_IMMEDIATE) {
//                    dmaActivate(id);
                }
            }
            break;
        }
    }
}
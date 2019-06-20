/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "cpu.hpp"

using namespace ARM;
using namespace NanoboyAdvance::GBA;

/* Retrieves DMA with highest priority from a DMA bitset. */
static constexpr int g_dma_from_bitset[] = {
    /* 0b0000 */ -1,
    /* 0b0001 */  0,
    /* 0b0010 */  1,
    /* 0b0011 */  0,
    /* 0b0100 */  2,
    /* 0b0101 */  0,
    /* 0b0110 */  1,
    /* 0b0111 */  0,
    /* 0b1000 */  3,
    /* 0b1001 */  0,
    /* 0b1010 */  1,
    /* 0b1011 */  0,
    /* 0b1100 */  2,
    /* 0b1101 */  0,
    /* 0b1110 */  1,
    /* 0b1111 */  0
};

void CPU::ResetDMAs() {
    dma_hblank_mask = 0;
    dma_vblank_mask = 0;
    dma_run_set = 0;
    dma_current = 0;
    dma_interleaved = false;
    
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

            if (dma.time == DMA_HBLANK) {
                dma_hblank_mask |=  (1<<id);
                dma_vblank_mask &= ~(1<<id);
            } else if (dma.time == DMA_VBLANK) { 
                dma_hblank_mask &= ~(1<<id);
                dma_vblank_mask |=  (1<<id);
            } else {
                dma_hblank_mask &= ~(1<<id);
                dma_vblank_mask &= ~(1<<id);
            }
                
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
                    MarkDMAForExecution(id);
                }
            }
            break;
        }
    }
}

void CPU::MarkDMAForExecution(int id) {
    // Defer execution of immediate DMA if another higher priority DMA is still running.
    // Otherwise go ahead at set is as the currently running DMA.
    if (dma_run_set == 0) {
        dma_current = id;
    } else if (id < dma_current) {
        dma_current = id;
        dma_interleaved = true;
    }

    // Mark DMA as enabled.
    dma_run_set |= (1 << id);
}

void CPU::TriggerHBlankDMA() {
//    for (int i = 0; i < 4; i++) {
//        auto& dma = mmio.dma[i];
//        if (dma.enable && dma.time == DMA_HBLANK) {
//            MarkDMAForExecution(i);
//        }
//    }
    int hblank_dma = g_dma_from_bitset[dma_run_set & dma_hblank_mask];
    
    if (hblank_dma >= 0)
        MarkDMAForExecution(hblank_dma);
}

void CPU::TriggerVBlankDMA() {
//    for (int i = 0; i < 4; i++) {
//        auto& dma = mmio.dma[i];
//        if (dma.enable && dma.time == DMA_VBLANK) {
//            MarkDMAForExecution(i);
//        }
//    }
    int vblank_dma = g_dma_from_bitset[dma_run_set & dma_vblank_mask];
    
    if (vblank_dma >= 0)
        MarkDMAForExecution(vblank_dma);
}

void CPU::RunDMA() {
    auto& dma = mmio.dma[dma_current];
    
    const auto src_cntl = dma.src_cntl;
    const auto dst_cntl = dma.dst_cntl;
    const bool words = dma.size == DMA_WORD;
    
    /* TODO: what happens if src_cntl equals DMA_RELOAD? */
    const int modify_table[2][4] = {
        { 2, -2, 0, 2 },
        { 4, -4, 0, 4 }
    };
    
    const int src_modify = modify_table[dma.size][src_cntl];
    const int dst_modify = modify_table[dma.size][dst_cntl];

    std::uint32_t word;
    
    /* Run DMA until completion or interruption. */
    if (words) {
        while (dma.internal.length != 0) {
            if (run_until <= 0) return;
            
            /* Stop if DMA was interleaved by higher priority DMA. */
            if (dma_interleaved) {
                dma_interleaved = false;
                return;
            }
            
            word = ReadWord(dma.internal.src_addr, ACCESS_SEQ);
            WriteWord(dma.internal.dst_addr, word, ACCESS_SEQ);
            
            dma.internal.src_addr += src_modify;
            dma.internal.dst_addr += dst_modify;
            dma.internal.length--;
        }
    } else {
        while (dma.internal.length != 0) {
            if (run_until <= 0) return;
            
            /* Stop if DMA was interleaved by higher priority DMA. */
            if (dma_interleaved) {
                dma_interleaved = false;
                return;
            }
            
            word = ReadHalf(dma.internal.src_addr, ACCESS_SEQ);
            WriteHalf(dma.internal.dst_addr, word, ACCESS_SEQ);
            
            dma.internal.src_addr += src_modify;
            dma.internal.dst_addr += dst_modify;
            dma.internal.length--;
        }
    }
    
    /* If this code path is reached, the DMA has completed. */
    
    if (dma.interrupt) {
        mmio.irq_if |= CPU::INT_DMA0 << dma_current;
    }
    
    if (dma.repeat) {
        /* Reload the internal length counter. */
        dma.internal.length = dma.length & s_dma_len_mask[dma_current];
        if (dma.internal.length == 0) {
            dma.internal.length = s_dma_len_mask[dma_current] + 1;
        }

        /* Reload destination address if specified. */
        if (dst_cntl == DMA_RELOAD) {
            dma.internal.dst_addr = dma.dst_addr & s_dma_dst_mask[dma_current];
        }

        /* If DMA is specified to be non-immediate, wait for it to be retriggered. */
        if (dma.time != DMA_IMMEDIATE) {
            dma_run_set &= ~(1 << dma_current);
        }
    } else {
        dma.enable = false;
        dma_run_set &= ~(1 << dma_current);
    }
    
    if (dma_run_set > 0) {
        dma_current = g_dma_from_bitset[dma_run_set];
    }
}
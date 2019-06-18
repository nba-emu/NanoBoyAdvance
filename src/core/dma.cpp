/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "cpu.hpp"

using namespace ARM;
using namespace NanoboyAdvance::GBA;

void CPU::ResetDMAs() {
    dma_running = 0;
    dma_current = 0;
    dma_loop_exit = false;
    
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

  void CPU::dmaActivate(int id) {
    // Defer execution of immediate DMA if another higher priority DMA is still running.
    // Otherwise go ahead at set is as the currently running DMA.
    if (dma_running == 0) {
        dma_current = id;
    } else if (id < dma_current) {
        dma_current = id;
        dma_loop_exit = true;
    }

    // Mark DMA as enabled.
    dma_running |= (1 << id);
}

void CPU::dmaFindHBlank() {
    for (int i = 0; i < 4; i++) {
        auto& dma = mmio.dma[i];
        if (dma.enable && dma.time == DMA_HBLANK) {
            dmaActivate(i);
        }
    }
}

void CPU::dmaFindVBlank() {
    for (int i = 0; i < 4; i++) {
        auto& dma = mmio.dma[i];
        if (dma.enable && dma.time == DMA_VBLANK) {
            dmaActivate(i);
        }
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
                    dmaActivate(id);
                }
            }
            break;
        }
    }
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
            if (dma_loop_exit) {
                dma_loop_exit = false;
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
            if (dma_loop_exit) {
                dma_loop_exit = false;
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
            dma_running &= ~(1 << dma_current);
        }
    } else {
        dma.enable = false;
        dma_running &= ~(1 << dma_current);
    }
    
    if (dma_running == 0) return;

    /* Find the next DMA to execute. 
     * TODO: maybe this can be optimized using a LUT.
     */
    for (int id = 0; id < 4; id++) {
        if (dma_running & (1 << id)) {
            dma_current = id;
            break;
        }
    }
}
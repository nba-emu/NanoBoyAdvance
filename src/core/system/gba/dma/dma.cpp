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
    void Emulator::dmaActivate(int id) {
        // Defer execution of immediate DMA if another higher priority DMA is still running.
        // Otherwise go ahead at set is as the currently running DMA.
        if (dma_running == 0) {
            dma_current = id;
        }
        else if (id < dma_current) {
            dma_current = id;
            dma_loop_exit = true;
        }

        // Mark DMA as enabled.
        dma_running |= (1 << id);
    }

    void Emulator::dmaFindHBlank() {
        for (int i = 0; i < 4; i++) {
            auto& dma = regs.dma[i];
            if (dma.enable && dma.time == DMA_HBLANK) {
                dmaActivate(i);
            }
        }
    }

    void Emulator::dmaFindVBlank() {
        for (int i = 0; i < 4; i++) {
            auto& dma = regs.dma[i];
            if (dma.enable && dma.time == DMA_VBLANK) {
                dmaActivate(i);
            }
        }
    }

    void Emulator::dmaTransfer() {
        auto& dma = regs.dma[dma_current];

        const auto src_cntl = dma.src_cntl;
        const auto dst_cntl = dma.dst_cntl;
        const bool words    = dma.size == DMA_WORD;

        // TODO: find out what happens if src_cntl is DMA_RELOAD.
        const int modify_table[2][4] = {
            { 2, -2, 0, 2 },
            { 4, -4, 0, 4 }
        };

        const int src_modify = modify_table[dma.size][src_cntl];
        const int dst_modify = modify_table[dma.size][dst_cntl];

        // Run DMA for as long as possible.
        if (words) {
            while (dma.internal.length != 0) {
                if (cycles_left <= 0) return;

                // Halt if DMA was interrupted by (higher priority) DMA.
                if (dma_loop_exit) {
                    dma_loop_exit = false;
                    return;
                }

                write32(dma.internal.dst_addr, read32(dma.internal.src_addr, M_DMA), M_DMA);
                dma.internal.src_addr += src_modify;
                dma.internal.dst_addr += dst_modify;
                dma.internal.length--;
            }
        }
        else {
            while (dma.internal.length != 0) {
                if (cycles_left <= 0) return;

                // Halt if DMA was interrupted by (higher priority) DMA.
                if (dma_loop_exit) {
                    dma_loop_exit = false;
                    return;
                }

                write16(dma.internal.dst_addr, read16(dma.internal.src_addr, M_DMA), M_DMA);
                dma.internal.src_addr += src_modify;
                dma.internal.dst_addr += dst_modify;
                dma.internal.length--;
            }
        }

        // If this code is reached, the DMA has completed. Trigger IRQ if wanted.
        if (dma.interrupt) {
            m_interrupt.request((InterruptType)(INTERRUPT_DMA_0 << dma_current));
        }

        if (dma.repeat) {
            // Reload the internal length counter.
            dma.internal.length = dma.length & s_dma_len_mask[dma_current];

            if (dma.internal.length == 0) dma.internal.length = s_dma_len_mask[dma_current] + 1;

            // Must reload destination address too if wanted.
            if (dst_cntl == DMA_RELOAD) {
                dma.internal.dst_addr = dma.dst_addr & s_dma_dst_mask[dma_current];
            }

            // Check for non-immediate DMA. Must wait for the DMA to get activated again.
            if (dma.time != DMA_IMMEDIATE) {
                dma_running &= ~(1 << dma_current);
            }
        }
        else {
            dma.enable   = false;
            dma_running &= ~(1 << dma_current);
        }

        // Skip DMA selection loop if no DMA is activated.
        if (dma_running == 0) return;

        // Select next DMA to execute.
        for (int id = 0; id < 4; id++) {
            if (dma_running & (1 << id)) {
                dma_current = id;
                break;
            }
        }
    }

    // TODO: FIFO DMA currently ignores DMA priority I think. This should not be the case.
    void Emulator::dmaTransferFIFO(int dma_id) {
        auto& dma = regs.dma[dma_id];

        for (int i = 0; i < 4; i++) {
            // transfer word to FIFO
            write32(dma.internal.dst_addr, read32(dma.internal.src_addr, M_DMA), M_DMA);

            // advance source address
            dma.internal.src_addr += 4;
        }

        if (dma.interrupt)
            m_interrupt.request((InterruptType)(INTERRUPT_DMA_0 << dma_id));
    }
}

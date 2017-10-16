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
        if (dma_running == 0 || id < dma_current) {
            dma_current = id;
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

        auto src_cntl = dma.src_cntl;
        auto dst_cntl = dma.dst_cntl;
        bool words    = dma.size == DMA_WORD;

        while (dma.internal.length != 0) {
            // Do not underrun the cycle counter.
            if (cycles_left <= 0) return;

            if (words) {
                write32(dma.internal.dst_addr, read32(dma.internal.src_addr, M_DMA), M_DMA);
            } else {
                write16(dma.internal.dst_addr, read16(dma.internal.src_addr, M_DMA), M_DMA);
            }

            if (dma.dst_cntl == DMA_INCREMENT || dma.dst_cntl == DMA_RELOAD) {
                dma.internal.dst_addr += dma.size == DMA_WORD ? 4 : 2;
            } else if (dma.dst_cntl == DMA_DECREMENT) {
                dma.internal.dst_addr -= dma.size == DMA_WORD ? 4 : 2;
            }

            if (dma.src_cntl == DMA_INCREMENT || dma.src_cntl == DMA_RELOAD) {
                dma.internal.src_addr += dma.size == DMA_WORD ? 4 : 2;
            } else if (dma.src_cntl == DMA_DECREMENT) {
                dma.internal.src_addr -= dma.size == DMA_WORD ? 4 : 2;
            }

            dma.internal.length--;
        }

        if (dma.repeat) {
            // TODO(accuracy): length, dst_addr must be masked.
            dma.internal.length = dma.length;

            if (dst_cntl == DMA_RELOAD) {
                dma.internal.dst_addr = dma.dst_addr;
            }

            if (dma.time != DMA_IMMEDIATE) {
                dma_running &= ~(1 << dma_current);
            }
        } else {
            dma.enable   = false;
            dma_running &= ~(1 << dma_current);
        }

        if (dma.interrupt)
            m_interrupt.request((InterruptType)(INTERRUPT_DMA_0 << dma_current));

        // Lookup next DMA to run, if any.
        if (dma_running != 0) {
            for (int id = 0; id < 4; id++) {
                // Check if the DMA is currently activated.
                if (dma_running & (1 << id)) {
                    dma_current = id;
                    break;
                }
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

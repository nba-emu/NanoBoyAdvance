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

#include "emulator.hpp"
#include "util/logger.hpp"

using namespace Util;

namespace GameBoyAdvance {
    
    void Emulator::dma_hblank() {
        for (int i = 0; i < 4; i++) {
            auto& dma = m_io.dma[i];

            if (dma.enable && dma.time == DMA_HBLANK) {
                m_dma_active  = true;
                m_current_dma = i;
                return;
            }
        }
    }

    void Emulator::dma_vblank() {
        for (int i = 0; i < 4; i++) {
            auto& dma = m_io.dma[i];

            if (dma.enable && dma.time == DMA_VBLANK) {
                m_dma_active  = true;
                m_current_dma = i;
                return;
            }
        }
    }

    void Emulator::dma_transfer() {
        auto& dma = m_io.dma[m_current_dma];

        DMAControl src_cntl = dma.src_cntl;
        DMAControl dst_cntl = dma.dst_cntl;
        bool words = dma.size == DMA_WORD;

        while (dma.internal.length != 0) {
            // do not desync...
            if (m_cycles < 0) {
                return;
            }

            if (words) {
                write_word(dma.internal.dst_addr, read_word(dma.internal.src_addr, MEM_NONE), MEM_NONE);
            } else {
                write_hword(dma.internal.dst_addr, read_hword(dma.internal.src_addr, MEM_NONE), MEM_NONE);
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

            // even though DMA will be repeated, we have to wait for it to be rescheduled.
            if (dma.time != DMA_IMMEDIATE) {
                m_dma_active  = false;
                m_current_dma = -1;
            }
        } else {
            dma.enable = false;
            m_dma_active = false;
            m_current_dma = -1;
        }

        if (dma.interrupt) {
            m_interrupt.request((InterruptType)(INTERRUPT_DMA_0 << dma.id));
        }
    }
    
    void Emulator::dma_fill_fifo(int dma_id) {
        auto& dma = m_io.dma[dma_id];
        
        for (int i = 0; i < 4; i++) {
            u32 value = read_word(dma.internal.src_addr, MEM_NONE);
            
            write_word(dma.internal.dst_addr, value, MEM_NONE);
            
            dma.internal.src_addr += 4;
        }
        
        if (dma.interrupt) {
            m_interrupt.request((InterruptType)(INTERRUPT_DMA_0 << dma.id));
        }
    }
}

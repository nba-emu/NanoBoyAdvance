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

#include "regs.hpp"
#include "../emulator.hpp"
#include "../memory/mmio.hpp"
#include "util/logger.hpp"

using namespace Util;

namespace Core {

    void Emulator::timerStep(int cycles) {
        if (regs.timer[0].control.enable) {
            timerRunInternal<0>(cycles);
        }
        if (regs.timer[1].control.enable) {
            timerRunInternal<1>(cycles);
        }
        if (regs.timer[2].control.enable) {
            timerRunInternal<2>(cycles);
        }
        if (regs.timer[3].control.enable) {
            timerRunInternal<3>(cycles);
        }
    }

    template <int id>
    void Emulator::timerRunInternal(int cycles) {
        auto& timer   = regs.timer[id];
        auto& control = timer.control;

        if (control.cascade) {
            if (id != 0 && regs.timer[id - 1].overflow) {
                timer.overflow = false;
                if (timer.counter != 0xFFFF) {
                    timer.counter++;
                }
                else {
                    timer.counter  = timer.reload;
                    timer.overflow = true;
                    if (control.interrupt) {
                        m_interrupt.request((InterruptType)(INTERRUPT_TIMER_0 << id));
                    }
                    if (timer.id < 2 && apu.getIO().control.master_enable) {
                        timerHandleFIFO(id, 1);
                    }
                }
                regs.timer[id - 1].overflow = false;
            }
        }
        else {
            int available = timer.cycles + cycles;
            int overflows = 0;

            timer.overflow = false;
            
            while (available >= timer.ticks) {
                if (timer.counter != 0xFFFF) {
                    timer.counter++;
                }
                else {
                    timer.counter  = timer.reload;
                    timer.overflow = true;
                    overflows++;
                }
                available -= timer.ticks;
            }
            if (timer.overflow) {
                if (control.interrupt) {
                    m_interrupt.request((InterruptType)(INTERRUPT_TIMER_0 << id));
                }
                if (timer.id < 2 && apu.getIO().control.master_enable) {
                    timerHandleFIFO(id, overflows);
                }
            }
            timer.cycles = available;
        }
    }

    void Emulator::timerHandleFIFO(int timer_id, int times) {
        auto& apu_io  = apu.getIO();
        auto& control = apu_io.control;

        static const u32 fifo_addr[2] = { FIFO_A, FIFO_B };

        for (int fifo = 0; fifo < 2; fifo++) {
            if (control.dma[fifo].timer_num != timer_id) {
                continue;
            }
            u32  address = fifo_addr[fifo];
            for (int time = 0; time < times; time++) {
                apu.signalFifoTransfer(fifo);
                if (!apu_io.fifo[fifo].requiresData()) {
                    continue;
                }
                for (int dma_id = 1; dma_id <= 2; dma_id++) {
                    auto& dma = regs.dma[dma_id];
                    if (dma.enable && dma.time == DMA_SPECIAL && dma.dst_addr == address) {
                        dmaTransferFIFO(dma_id);
                        break;
                    }
                }
            }
        }
    }

}

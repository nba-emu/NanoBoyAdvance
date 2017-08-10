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

namespace GameBoyAdvance {

    // cycles required for one increment
    static constexpr int g_timer_ticks[4] = { 1, 64, 256, 1024 };

    // shift amount and mask for division/modulo by total_cycles
    static constexpr int g_timer_shift[4] = { 0, 6, 8, 10 };
    static constexpr int g_timer_mask [4] = { 0, 0x3F, 0xFF, 0x3FF };

    void Emulator::timerStep(int cycles) {
        for (int i = 0; i < 4; i++) {
            auto& timer   = regs.timer[i];
            auto& control = timer.control;

            // handles only non-cascade timers directly
            if (control.enable && !control.cascade) {
                int cycles_left   = cycles;
                int total_cycles  = g_timer_ticks[control.frequency];
                int needed_cycles = total_cycles - timer.cycles;

                // does the cycle amount satifies an increment?
                if (cycles_left >= needed_cycles) {
                    int increments = 1; // atleast one increment

                    // consume the amount of cycles needed for the first increment
                    cycles_left -= needed_cycles;

                    // check if more increments are still possible
                    if (cycles_left >= total_cycles) {
                        if (control.frequency == 0) {
                            // in this case frequency = F/1 and we can take a shortcut
                            increments += cycles_left;
                            cycles_left = 0;
                        } else {
                            // divide left amount of cycles to run by the total cycles needed for an increment
                            // to calculate: a) furtherly satisfyable increments (quotient) and
                            //               b) amount of cycles left after doing them (remainder)
                            increments += cycles_left >> g_timer_shift[control.frequency];
                            cycles_left = cycles_left  & g_timer_mask [control.frequency];
                        }
                    }

                    // increment timer by the calculated amount
                    timerIncrement(timer, increments);
                }

                // don't loose the left amount of cycles
                timer.cycles += cycles_left;
            }
        }
    }

    void Emulator::timerHandleFIFO(int timer_id, int times) {
        auto& apu_io  = apu.getIO();
        auto& control = apu_io.control;
        auto& dma1    = regs.dma[1];
        auto& dma2    = regs.dma[2];

        // for each DMA FIFO
        for (int i = 0; i < 2; i++) {

            // is the overflowing timer responsible for this FIFO?
            if (control.dma[i].timer_num == timer_id) {
                auto& fifo = apu_io.fifo[i];

                for (int j = 0; j < times; j++) {
                    // transfers sample from FIFO to APU chip
                    apu.signalFifoTransfer(i);

                    // tries to trigger DMA transfer if FIFO requests more data
                    if (fifo.requiresData()) {

                        u32 address = (i == 0) ? FIFO_A : FIFO_B;

                        if (dma1.enable && dma1.time == DMA_SPECIAL && dma1.dst_addr == address) {
                            dmaTransferFIFO(1);
                        } else if (dma2.enable && dma2.time == DMA_SPECIAL && dma2.dst_addr == address) {
                            dmaTransferFIFO(2);
                        }
                    }
                }
            }
        }
    }

    void Emulator::timerHandleOverflow(Timer& timer, int times) {
        // request timer overflow interrupt if needed
        if (timer.control.interrupt) {
            m_interrupt.request((InterruptType)(INTERRUPT_TIMER_0 << timer.id));
        }

        // reload counter value
        timer.counter = timer.reload;

        // cascade next timer if required
        if (timer.id != 3) {
            auto& timer2 = regs.timer[timer.id + 1];

            if (timer2.control.enable && timer2.control.cascade) {
                if (times == 1) {
                    timerIncrementOnce(timer2);
                } else {
                    timerIncrement(timer2, times);
                }
            }
        }

        // handle FIFO transfer if sound is enabled
        if (apu.getIO().control.master_enable) {
            timerHandleFIFO(timer.id, times);
        }
    }

    void Emulator::timerIncrement(Timer& timer, int increment_count) {

        int next_overflow = 0x10000 - timer.counter;

        // reset the timer's cycle counter
        timer.cycles = 0;

        // does overflow happen at all?
        if (increment_count >= next_overflow) {
            int overflow_count = 1;

            // "eat" the increments for the first overflow
            increment_count -= next_overflow;

            // if there are increments left
            if (increment_count != 0) {
                int required = 0x10000 - timer.reload;

                overflow_count += increment_count / required;
                increment_count = increment_count % required;
            }

            timerHandleOverflow(timer, overflow_count);
        }

        // update the counter with the remaining increments
        timer.counter += increment_count;
    }

    // Short-cut method for cascade timers that ALWAYS get incremented by one.
    void Emulator::timerIncrementOnce(Timer& timer) {
        if (timer.counter != 0xFFFF) {
            timer.counter++;
        } else {
            timerHandleOverflow(timer, 1);
        }
    }
}

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
#include <fstream>
#include <string>

using namespace Util;

namespace GameBoyAdvance {
    
    constexpr int CPU::m_timer_ticks[4];

    void CPU::timer_step(int cycles) {
        bool overflow = false;

        for (int i = 0; i < 4; ++i) {
            auto& timer = m_io.timer[i];

            if (timer.control.enable) {
                if (timer.control.cascade && overflow) {
                    
                    timer_increment(timer, overflow);
                
                } else if (!timer.control.cascade) {
                    timer.ticks += cycles;

                    // optimize. m_timer_ticks creates an actual lookup.
                    if (timer.ticks >= m_timer_ticks[timer.control.frequency]) {
                        timer_increment(timer, overflow);
                    } else {
                        overflow = false;
                    }
                } else {
                    overflow = false;
                }
            } else {
                overflow = false;
            }
        }
    }

    void CPU::timer_increment(struct IO::Timer& timer, bool& overflow) {
        timer.ticks = 0;

        if (timer.counter == 0xFFFF) {
            if (timer.control.interrupt) {
                m_interrupt.request((InterruptType)(INTERRUPT_TIMER_0 << timer.id));
            }
            overflow = true;
            timer.counter = timer.reload;
            
            // DMA FIFO handling
            auto& apu_io  = m_apu.get_io();
            auto& control = apu_io.control;
            auto& dma1    = m_io.dma[1];
            auto& dma2    = m_io.dma[2];
            
            if (control.master_enable) {
                for (int i = 0; i < 2; i++) {
                    if (control.dma[i].timer_num == timer.id) {
                        auto& fifo = apu_io.fifo[i];
                        
                        std::string filename = (i == 0) ? "fifo_a.raw" : "fifo_b.raw";
                        std::ofstream s(filename, std::ofstream::out | std::ofstream::binary | std::ofstream::app);
                        
                        s8 sample = fifo.dequeue();
                        fmt::print("fifo{1}: {0:x}\n", sample, i);
                        s << sample;
                        s.close();
                        
                        if (fifo.requires_data()) {
                            u32 address = (i == 0) ? 0x040000A0 : 0x040000A4; // eh...
                            
                            if (dma1.time == DMA_SPECIAL && dma1.dst_addr == address) {
                                dma_fill_fifo(1);
                            } else if (dma2.time == DMA_SPECIAL && dma2.dst_addr == address) {
                                dma_fill_fifo(2);
                            }
                        }
                    }
                }
            }
        } else {
            overflow = false;
            timer.counter++;
        }
    }
}

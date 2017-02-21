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
#include "mmio.hpp"
#include "util/logger.hpp"
//#include <fstream>
//#include <string>

using namespace Util;

namespace GameBoyAdvance {
    
    constexpr int CPU::m_timer_ticks[4];

    void CPU::timer_step(int cycles) {
        
        for (int i = 0; i < 4; i++) {
            auto& timer   = m_io.timer[i];
            auto& control = timer.control;
            
            if (control.enable && !control.cascade) {
                int cycles_left   = cycles;
                int total_cycles  = m_timer_ticks[control.frequency];
                int needed_cycles = total_cycles - timer.cycles;

                if (cycles_left >= needed_cycles) {
                    int increments = 1;
                    
                    cycles_left -= needed_cycles;

                    if (cycles_left >= total_cycles) {
                        increments += cycles_left / total_cycles;
                        cycles_left = cycles_left % total_cycles;
                    }
                    
                    timer_increment(i, increments);
                }
                
                timer.cycles += cycles_left;
            }
        }
    }

    void CPU::timer_increment(int timer_id, int increment_count) {

        auto& timer         = m_io.timer[timer_id];
        int   next_overflow = 0x10000 - timer.counter;
        
        timer.cycles = 0;
        
        while (increment_count >= next_overflow) {
            
            if (timer.control.interrupt) {
                m_interrupt.request((InterruptType)(INTERRUPT_TIMER_0 << timer_id));
            }
            
            timer.counter = timer.reload;
            
            if (timer_id != 3) {
                auto& timer2 = m_io.timer[timer_id + 1];

                if (timer2.control.enable && timer2.control.cascade) {
                    timer_increment_once(timer2);
                }
            }
            
            if (m_apu.get_io().control.master_enable) {
                timer_fifo(timer.id);        
            }
            
            increment_count -= next_overflow;
            
            next_overflow = 0x10000 - timer.counter;
        }
        
        timer.counter += increment_count;
    }
    
    void CPU::timer_increment_once(IO::Timer& timer) {
        
        if (timer.counter == 0xFFFF) {
            timer.counter++;    
        } else {
            if (timer.control.interrupt) {
                m_interrupt.request((InterruptType)(INTERRUPT_TIMER_0 << timer.id));
            }
            
            timer.counter = timer.reload;
            
            if (timer.id != 3) {
                auto& timer2 = m_io.timer[timer.id + 1];

                if (timer2.control.enable && timer2.control.cascade) {
                    timer_increment_once(timer2);
                }
            }
            
            if (m_apu.get_io().control.master_enable) {
                timer_fifo(timer.id);        
            }
        }
    }
    
    void CPU::timer_fifo(int timer_id) {
        auto& apu_io  = m_apu.get_io();
        auto& control = apu_io.control;
        auto& dma1    = m_io.dma[1];
        auto& dma2    = m_io.dma[2];
        
        for (int i = 0; i < 2; i++) {
            
            if (control.dma[i].timer_num == timer_id) {
                auto& fifo = apu_io.fifo[i];
                        
                m_apu.fifo_get_sample(i);
                        
                if (fifo.requires_data()) {
                    u32 address = (i == 0) ? FIFO_A : FIFO_B;
                        
                    if (dma1.enable && dma1.time == DMA_SPECIAL && dma1.dst_addr == address) {
                        dma_fill_fifo(1);
                    } else if (dma2.enable && dma2.time == DMA_SPECIAL && dma2.dst_addr == address) {
                        dma_fill_fifo(2);
                    }
                }
            }
        }
    }
    
}

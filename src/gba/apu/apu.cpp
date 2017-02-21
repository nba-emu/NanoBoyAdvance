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

#include "apu.hpp"

namespace GameBoyAdvance {
    
    APU::APU() {
        // forward FIFO access to SOUNDCNT register (for FIFO resetting)
        m_io.control.fifo[0] = &m_io.fifo[0];
        m_io.control.fifo[1] = &m_io.fifo[1];
        
        // reset state
        reset();
    }
    
    void APU::reset() {
        m_io.fifo[0].reset();
        m_io.fifo[1].reset();
        m_io.bias.reset();
        m_io.control.reset();
    }
    
    void APU::fill_buffer(s8* stream, int length) {
        
        double ratio = 13389.0 / 44100.0;
        
        m_mutex.lock();
        
        length /= 2; // length is in bytes plus 2 channels..
        
        for (int i = 0; i < length; i++) {
            if ((i * ratio) >= m_fifo_buffer[0].size()) {
                stream[i * 2] = 0;
            } else {
                stream[i * 2] = m_fifo_buffer[0][i * ratio];
            }
            
            if ((i * ratio)  >= m_fifo_buffer[1].size()) {
                stream[i * 2 + 1] = 0;
            } else {
                stream[i * 2 + 1] = m_fifo_buffer[1][i * ratio];
            }
        }
        
        int actual_length = length * ratio;
        
        if (actual_length >= m_fifo_buffer[0].size()) {
            m_fifo_buffer[0].clear();
        } else {
            m_fifo_buffer[0].erase(m_fifo_buffer[0].begin(), m_fifo_buffer[0].begin()+actual_length);
        }
        
        if (actual_length >= m_fifo_buffer[1].size()) {
            m_fifo_buffer[1].clear();
        } else {
            m_fifo_buffer[1].erase(m_fifo_buffer[1].begin(), m_fifo_buffer[1].begin()+actual_length);
        }
        
        m_mutex.unlock();
    }
}
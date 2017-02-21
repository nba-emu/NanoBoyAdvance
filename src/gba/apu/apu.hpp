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

#pragma once

#include "fifo.hpp"
#include "util/integer.hpp"
#include <vector>
#include <mutex>

#define APU_INCLUDE

namespace GameBoyAdvance {
    
    class APU {
    private:
        
        #include "io.hpp"
    
        std::mutex m_mutex;
        
        std::vector<s8> m_fifo_buffer[2];
        
    public:
        APU();
        
        void reset();
        
        IO& get_io() {
            return m_io;
        }
        
        void fill_buffer(s8* stream, int length);
        
        void fifo_get_sample(int fifo_id) {
            auto& fifo   = m_io.fifo[fifo_id];
            auto& buffer = m_fifo_buffer[fifo_id];
            
            m_mutex.lock();
            buffer.push_back(fifo.dequeue());
            m_mutex.unlock();
        }
    };
}

#undef APU_INCLUDE
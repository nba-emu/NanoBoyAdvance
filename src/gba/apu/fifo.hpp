/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
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

#include <cstdint>

namespace GameBoyAdvance {

class FIFO {   
public:
    FIFO() { Reset(); }
    
    void Reset() {
        rd_ptr = 0;
        wr_ptr = 0;
        count = 0;
    }
    
    int Count() const { return count; }
    
    void Write(std::int8_t sample) {
        if (count < s_fifo_len) {
            data[wr_ptr] = sample;
            wr_ptr = (wr_ptr + 1) % s_fifo_len;
            count++;
        }
    }
    
    auto Read() -> std::int8_t {
        std::int8_t value = 0;
        
        if (count > 0) {
            value = data[rd_ptr];
            rd_ptr = (rd_ptr + 1) % s_fifo_len;
            count--;
        }
        
        return value;
    }

private:
    static constexpr int s_fifo_len = 32;
    
    std::int8_t data[s_fifo_len];
 
    int rd_ptr;
    int wr_ptr;
    int count;
};
    
}
    
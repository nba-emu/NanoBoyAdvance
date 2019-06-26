/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>

namespace NanoboyAdvance::GBA {

class FIFO {   
public:
    FIFO() { Reset(); }
    
    void Reset() {
        rd_ptr = 0;
        wr_ptr = 0;
        free = s_fifo_len;
    }
    
    void Write(std::int8_t sample) {
        if (free > 0) {
            data[wr_ptr] = sample;
            wr_ptr = (wr_ptr + 1) % s_fifo_len;
            free--;
        }
    }
    
    std::int8_t Read() {
        std::int8_t value = 0;
        
        if (free < s_fifo_len) {
            value = data[rd_ptr];
            rd_ptr = (rd_ptr + 1) % s_fifo_len;
            free++;
        }
        
        return value;
    }

private:
    static constexpr int s_fifo_len = 32;
    
    std::int8_t data[s_fifo_len];
 
    int rd_ptr;
    int wr_ptr;
    int free;
};
    
}
    
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
#include "../config.hpp"
#include "util/integer.hpp"
#include <vector>
#include <mutex>

#define APU_INCLUDE

namespace GameBoyAdvance {
    
    class APU {
    private:
        
        #include "io.hpp"
    
        static constexpr float s_wave_duty[4] = { 0.125, 0.25, 0.5, 0.75 };
        static constexpr int s_sweep_clock[8] = {
            0, 130884, 261768, 392652, 523536, 654420, 785304, 916188
        };
        static constexpr int s_envelope_clock[8] = {
            0, 262187, 524375, 786562, 1048750, 1310937, 1573125, 1835312
        };
        static constexpr float s_psg_volume[] = { 0.25, 0.5, 1, 1 };
        static constexpr float s_dma_volume[] = { 2, 4 };
        static constexpr float s_wav_volume[] = { 0, 1, 0.5, 0.25 };
        
        std::mutex m_mutex;
        std::vector<s8> m_psg_buffer[2];
        std::vector<s8> m_fifo_buffer[2];
        
        int m_cycle_count { 0 };
        int m_sample_rate { 44100 };
        
        Config* m_config;
        
    public:
        APU(Config* config);
        
        void reset();
        void load_config();
        
        IO& get_io() {
            return m_io;
        }
        
        static auto convert_frequency(int freq) -> float;
        
        auto generate_quad(int id) -> float;
        auto generate_wave() -> float;
        void update_quad(int step_cycles);
        void update_wave(int step_cycles);
        
        void mix_samples(int samples);
        
        void step(int step_cycles);
        
        void fill_buffer(u16* stream, int length);
        
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
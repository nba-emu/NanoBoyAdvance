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
        
        // Stores latched FIFO samples
        s8 m_fifo_sample[2];
        
        // Stereo output (ring buffers)
        u16 m_output[2][0x4000];
        int m_read_pos  { 0 };
        int m_write_pos { 0 };
                
        int m_cycle_count { 0 };
        int m_sample_rate { 44100 };
        
        Config* m_config;
        
    public:
        APU(Config* config);
        
        void reset();
        void reloadConfig();
        
        IO& getIO() {
            return m_io;
        }
        
        // Convert GBA frequency to real freq.
        static auto convertFrequency(int freq) -> float;
        
        // Sound Generators
        auto generateQuad(int id) -> float;
        auto generateWave()       -> float;
        auto generateNoise()      -> float;
        
        // Updates PSG states
        void updateQuad (int step_cycles);
        void updateWave (int step_cycles);
        void updateNoise(int step_cycles);
        
        // Mix all channels together
        void mixChannels(int samples);
        
        // Advance state by a given amount of cycles
        void step(int step_cycles);
        
        // Fill audio buffer from ring buffer
        void fillBuffer(u16* stream, int length);
        
        // Pull next sample from FIFO A (0) or B (1)
        void signalFifoTransfer(int fifo_id) {
            m_fifo_sample[fifo_id] = m_io.fifo[fifo_id].dequeue();
        }
    };
}

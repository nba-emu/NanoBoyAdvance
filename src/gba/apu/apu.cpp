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

#include <cmath>
#include "apu.hpp"

#include "util/logger.hpp"

namespace GameBoyAdvance {
    
    #ifndef M_PI
    #define M_PI (3.14159265358979323846)
    #endif
    
    constexpr float APU::s_wave_duty[4];
    constexpr int   APU::s_sweep_clock[8];
    constexpr int   APU::s_envelope_clock[8];
    constexpr float APU::s_psg_volume[4];
    constexpr float APU::s_dma_volume[2];
    constexpr float APU::s_wav_volume[4];
    
    APU::APU(Config* config) : m_config(config) {
        
        // forward FIFO access to SOUNDCNT register (for FIFO resetting)
        m_io.control.fifo[0] = &m_io.fifo[0];
        m_io.control.fifo[1] = &m_io.fifo[1];
        
        // reset state
        reset();
    }
    
    void APU::reset() {
        m_io.fifo[0].reset();
        m_io.fifo[1].reset();
        m_io.tone[0].reset();
        m_io.tone[1].reset();
        m_io.wave.reset();
        
        m_io.bias.reset();
        m_io.control.reset();
    }
    
    void APU::load_config() {
        
    }
    
    auto APU::convert_frequency(int freq) -> float {
        return 131072 / (2048 - freq);
    }
    
    auto APU::generate_quad(int id) -> float {
        
        auto& channel  = m_io.tone[id];
        auto& internal = channel.internal;
        auto& cycles   = internal.cycles;
        
        if (channel.apply_length) {
            int max_cycles = (64 - channel.sound_length) * (1.0 / 256.0) * 16780000;
            
            if (cycles.length >= max_cycles) {
                return 0;
            }
        }

        float amplitude = (float)internal.volume * (1.0 / 16.0);
        float frequency = convert_frequency(internal.frequency);
        float position  = (float)((2 * M_PI * internal.sample * frequency) / m_sample_rate);
        float wave_duty = s_wave_duty[channel.wave_duty];
        
        float value = 0;
        
        // generates sample based upon wave duty and sample position
        if (std::fmod(position, 2 * M_PI) <= 2 * M_PI * wave_duty) {
            value = 127;
        } else {
            value = -128;
        }
        
        if (++internal.sample >= m_sample_rate) {
            internal.sample = 0;
        }
        
        return amplitude * value;
    }
    
    auto APU::generate_wave() -> float {
        
        auto& wave     = m_io.wave;
        auto& wave_int = wave.internal; 
        
        if (!wave.playback) {
            return 0;
        }
        
        if (wave.apply_length) {
            int max_cycles = (256 - wave.sound_length) * (1.0 / 256.0) * 16780000;
            
            if (wave_int.length_cycles >= max_cycles) {
                return 0;
            }
        }
         
        u8  byte   = m_io.wave_ram[wave.bank_number][wave_int.sample_ptr >> 1];
        int sample = (wave_int.sample_ptr & 1) ? (byte & 15) : (byte >> 4);
        
        float volume = wave.force_volume ? 0.75 : s_wav_volume[wave.volume];
        
        return (sample - 8) * volume * 8;
    }
    
    void APU::update_quad(int step_cycles) {
        
        for (int i = 0; i < 2; i++) {
            
            auto& channel  = m_io.tone[i];
            auto& internal = channel.internal;
            auto& cycles   = internal.cycles;
            auto& sweep    = channel.sweep;
            auto& envelope = channel.envelope;
            
            if (sweep.time != 0) {
                int sweep_clock = s_sweep_clock[sweep.time];
                
                cycles.sweep += step_cycles;
                
                // not very optimized - i suppose
                while (cycles.sweep >= sweep_clock) {
                    int shift = internal.frequency >> sweep.shift;
                    
                    if (sweep.direction == SWEEP_INC) {
                        internal.frequency += shift;
                    } else {
                        internal.frequency -= shift;
                    }
                    
                    if (internal.frequency >= 2048) {
                        internal.frequency = 2047;
                    } else if (internal.frequency < 0) {
                        internal.frequency = 0;
                    }
                    
                    cycles.sweep -= sweep_clock;
                }
            }
            
            if (envelope.time != 0) {
                int envelope_clock = s_envelope_clock[envelope.time];
                
                cycles.envelope += step_cycles;
                
                // not very optimized - i suppose
                while (cycles.envelope >= envelope_clock) {
                    
                    if (envelope.direction == ENV_INC) {
                        if (internal.volume != 15) internal.volume++;
                    } else {
                        if (internal.volume != 0 ) internal.volume--;
                    }
                    
                    cycles.envelope -= envelope_clock;
                }
            }
            
            if (channel.apply_length) {
                cycles.length += step_cycles;
            }
        }
    }
    
    void APU::update_wave(int step_cycles) {
        auto& wave     = m_io.wave;
        auto& wave_int = wave.internal;
        
        // concert sample rate to cycles
        int required = 16780000 / (2097152 / (2048 - wave.frequency));
        
        wave_int.sample_cycles += step_cycles;
        
        if (wave_int.sample_cycles >= required) {
            int amount = wave_int.sample_cycles / required;
            
            wave_int.sample_ptr += amount;
            
            // check for wave RAM overflow
            if (wave_int.sample_ptr >= 32) {
                // wraparound sample pointer
                wave_int.sample_ptr &= 0x1F;
                
                // 64-digit mode?
                if (wave.dimension) {
                    // toggle bank number
                    wave.bank_number ^= 1;
                }    
            }
            
            // eat the cycles that were "consumed"
            wave_int.sample_cycles -= amount * required;
        }
        
        if (wave.apply_length) {
            wave.internal.length_cycles += step_cycles;
        }
    }
    
    void APU::mix_samples(int samples) {
        
        auto& psg = m_io.control.psg;
        
        for (int sample = 0; sample < samples; sample++) {
            s16 out1 = 0;
            s16 out2 = 0;
            
            float volume_left  = (float)psg.master[SIDE_LEFT ] / 7.0;
            float volume_right = (float)psg.master[SIDE_RIGHT] / 7.0;
            
            s16 tone1 = (s16)generate_quad(0);
            s16 tone2 = (s16)generate_quad(1);
            s16 wave  = (s16)generate_wave();
            
            // calculate left side
            if (psg.enable[SIDE_LEFT][0]) out1 += tone1;
            if (psg.enable[SIDE_LEFT][1]) out1 += tone2;
            if (psg.enable[SIDE_LEFT][2]) out1 += wave;
            
            // calculate right side
            if (psg.enable[SIDE_RIGHT][0]) out2 += tone1;
            if (psg.enable[SIDE_RIGHT][1]) out2 += tone2;
            if (psg.enable[SIDE_RIGHT][2]) out2 += wave;
            
            // prevent evil simultanious accesses
            m_mutex.lock();
            
            m_psg_buffer[SIDE_LEFT ].push_back(out1 * volume_left);
            m_psg_buffer[SIDE_RIGHT].push_back(out2 * volume_right);
            
            m_mutex.unlock();
            
        }
        
    }
    
    void APU::step(int step_cycles) {
        
        int cycles_per_sample = 16780000 / m_sample_rate;
        
        update_quad(step_cycles);
        update_wave(step_cycles);
        
        m_cycle_count += step_cycles;
        
        if (m_cycle_count >= cycles_per_sample) {
            int samples = m_cycle_count / cycles_per_sample;
            
            mix_samples(samples);
            
            // consume the cycles required for each sample
            m_cycle_count -= samples * cycles_per_sample;    
        }
        
    }
    
    void APU::fill_buffer(u16* stream, int length) {
                
        auto& psg  = m_io.control.psg;
        auto  dma  = m_io.control.dma;
        auto& bias = m_io.bias;
        
        float psg_volume = s_psg_volume[psg.volume];
        
        m_mutex.lock();
        
        // buffer size that we strech the FIFO samples to
        int actual_length = m_psg_buffer[0].size();
        
        int fifo_size[2] = {
            (int)m_fifo_buffer[0].size(),    
            (int)m_fifo_buffer[1].size()    
        };
        
        const float fifo_amplitude[2] = {
            APU::s_dma_volume[dma[0].volume],
            APU::s_dma_volume[dma[1].volume]
        };
                
        float ratio[2];
        
        // FIFO strechting ratios (need better mechanism)
        ratio[0] = (float)fifo_size[0] / (float)actual_length;
        ratio[1] = (float)fifo_size[1] / (float)actual_length;
        
        // handle fast forward. only keep 1/multiplier data of each buffer!
        // TODO: this is a bit hacked-in currently.
        //       we generate all the samples, just to drop most of them..
        if (m_config->fast_forward && m_config->multiplier > 1) {
            
            float ratio = 1 / (float)m_config->multiplier;
            
            // recalculate buffer sizes
            actual_length *= ratio;
            fifo_size[0]  *= ratio;
            fifo_size[1]  *= ratio;
            
            // shrink buffers
            m_psg_buffer[0].erase(m_psg_buffer[0].begin(), m_psg_buffer[0].begin() + actual_length);
            m_psg_buffer[1].erase(m_psg_buffer[1].begin(), m_psg_buffer[1].begin() + actual_length);
            m_fifo_buffer[0].erase(m_fifo_buffer[0].begin(), m_fifo_buffer[0].begin() + fifo_size[0]);
            m_fifo_buffer[1].erase(m_fifo_buffer[1].begin(), m_fifo_buffer[1].begin() + fifo_size[1]);
        }
        
        // divide length by four because:
        // 1) length is provided in bytes, while we work with hwords
        // 2) length is twice the actual length because of two stereo channels
        length >>= 2;
        
        s16   output[2];
        float source_index[2] { 0, 0 };
        
        for (int i = 0; i < length; i++) {
            
            // buffer range check
            if (i >= actual_length) {
                // TODO: prevent clicking
                stream[i * 2 + 0] = 0;
                stream[i * 2 + 1] = 0;
                continue;
            }
            
            // output PSG generated audio
            output[SIDE_LEFT ] = m_psg_buffer[SIDE_LEFT ][i] * psg_volume;
            output[SIDE_RIGHT] = m_psg_buffer[SIDE_RIGHT][i] * psg_volume;
            
            // add streched FIFO audio
            for (int fifo = 0; fifo < 2; fifo++) {
                
                // FIFO is mixed on neither side? do nothing
                if (!dma[fifo].enable[SIDE_LEFT] && 
                    !dma[fifo].enable[SIDE_RIGHT]) { continue; }
                
                int sample_index = std::round(source_index[fifo]);
                
                // prevent FIFO buffer overrun
                if (sample_index >= fifo_size[fifo]) {
                    sample_index = fifo_size[fifo];
                }
                
                s16 sample = m_fifo_buffer[fifo][sample_index] * fifo_amplitude[fifo];
                
                if (dma[fifo].enable[SIDE_LEFT]) {
                    output[SIDE_LEFT ] += sample;
                }
                if (dma[fifo].enable[SIDE_RIGHT]) {
                    output[SIDE_RIGHT] += sample;
                }
                
                source_index[fifo] += ratio[fifo];
            }
            
            // makeup gain (BIAS) and clipping emulation
            output[SIDE_LEFT ] += bias.level;
            output[SIDE_RIGHT] += bias.level;
            if (output[SIDE_LEFT ] < 0)     output[SIDE_LEFT ] = 0;
            if (output[SIDE_RIGHT] < 0)     output[SIDE_RIGHT] = 0;
            if (output[SIDE_LEFT ] > 0x3FF) output[SIDE_LEFT ] = 0x3FF;
            if (output[SIDE_RIGHT] > 0x3FF) output[SIDE_RIGHT] = 0x3FF;

            // reduce amplitude resolution
            output[SIDE_LEFT ] = (output[SIDE_LEFT ] >> bias.resolution) << bias.resolution;
            output[SIDE_RIGHT] = (output[SIDE_RIGHT] >> bias.resolution) << bias.resolution;

            stream[i * 2 + 0] = (u16)output[SIDE_LEFT ] * 64;
            stream[i * 2 + 1] = (u16)output[SIDE_RIGHT] * 64;
        }
        
        // remove used samples from the buffers
        if (actual_length > length) {
            
            /*if (m_fifo_buffer[0].size() > source_index[0]) {
                m_fifo_buffer[0].erase(m_fifo_buffer[0].begin(), m_fifo_buffer[0].begin()+source_index[0]);    
            }
            
            if (m_fifo_buffer[1].size() > source_index[1]) {
                m_fifo_buffer[1].erase(m_fifo_buffer[1].begin(), m_fifo_buffer[1].begin()+source_index[1]);    
            }
            
            m_psg_buffer[0].erase(m_psg_buffer[0].begin(), m_psg_buffer[0].begin()+length);
            m_psg_buffer[1].erase(m_psg_buffer[1].begin(), m_psg_buffer[1].begin()+length);*/
            auto& psg_buf = m_psg_buffer;
            auto& fifo_buf = m_fifo_buffer;
        
            for (int i = 0; i < 2; i++) {
                psg_buf[i].erase(psg_buf[i].begin(), psg_buf[i].end()-length);
                
                if (fifo_buf[i].size() > source_index[i]) {
                    int disp = length * ratio[i];
                    
                    fifo_buf[i].erase(fifo_buf[i].begin(), fifo_buf[i].end()-disp);
                }
            }
        } else {
            m_psg_buffer[0].clear();
            m_psg_buffer[1].clear();
            m_fifo_buffer[0].clear();
            m_fifo_buffer[1].clear();
        }
        
        m_mutex.unlock();
        
    }
}
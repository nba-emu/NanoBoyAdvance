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

namespace Core {
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
        regs.control.fifo[0] = &regs.fifo[0];
        regs.control.fifo[1] = &regs.fifo[1];

        // reset state
        reset();
    }

    void APU::reset() {
        // reset ringbuffer(s)
        read_pos  = 0;
        write_pos = 0;
        memset(ringbuffers[0], 0, 0x4000);
        memset(ringbuffers[1], 0, 0x4000);

        // reset channel (registers)
        for (int i = 0; i < 2; i++) {
            regs.fifo[i].reset();
            regs.tone[i].reset();
        }
        regs.wave.reset();
        regs.noise.reset();

        // reset control registers
        regs.bias.reset();
        regs.control.reset();
    }

    void APU::reloadConfig() {}

    auto APU::convertFrequency(int freq) -> float {
        return 131072 / (2048 - freq);
    }

    auto APU::generateQuad(int id) -> float {
        auto& channel  = regs.tone[id];
        auto& internal = channel.internal;
        auto& cycles   = internal.cycles;

        if (channel.apply_length) {
            int max_cycles = (64 - channel.sound_length) * (1.0 / 256.0) * 16780000;

            if (cycles.length >= max_cycles) {
                return 0;
            }
        }

        float amplitude = (float)internal.volume * (1.0 / 16.0);
        float frequency = convertFrequency(internal.frequency);
        float position  = (float)((2 * M_PI * internal.sample * frequency) / sample_rate);
        float wave_duty = s_wave_duty[channel.wave_duty];

        float value = 0;

        // generates sample based upon wave duty and sample position
        if (std::fmod(position, 2 * M_PI) <= 2 * M_PI * wave_duty) {
            value = 127;
        } else {
            value = -128;
        }

        if (++internal.sample >= sample_rate) {
            internal.sample = 0;
        }

        return amplitude * value;
    }

    auto APU::generateWave() -> float {
        auto& wave     = regs.wave;
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

        u8  byte   = regs.wave_ram[wave.bank_number][wave_int.sample_ptr >> 1];
        int sample = (wave_int.sample_ptr & 1) ? (byte & 15) : (byte >> 4);

        float volume = wave.force_volume ? 0.75 : s_wav_volume[wave.volume];

        return (sample - 8) * volume * 8;
    }

    auto APU::generateNoise() -> float {
        auto& noise     = regs.noise;
        auto& noise_int = noise.internal;

        if (noise.apply_length) {
            int max_cycles = (64 - noise.sound_length) * (1.0 / 256.0) * 16780000;

            if (noise_int.length_cycles >= max_cycles) {
                return 0;
            }
        }

        float amplitude = (float)noise_int.volume * (1.0 / 16.0);

        return amplitude * (noise_int.output ? 128 : -128);
    }

    void APU::updateQuad(int step_cycles) {
        for (int i = 0; i < 2; i++) {
            auto& channel  = regs.tone[i];
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

    void APU::updateWave(int step_cycles) {
        auto& wave     = regs.wave;
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
            wave_int.length_cycles += step_cycles;
        }
    }

    void APU::updateNoise(int step_cycles) {
        auto& noise     = regs.noise;
        auto& noise_int = noise.internal;
        auto& envelope  = noise.envelope;

        // TODO: make this less shitty
        int shift_freq = 524288;

        if (noise.divide_ratio == 0) {
            shift_freq <<= 1;
        } else {
            shift_freq /= noise.divide_ratio;
        }

        shift_freq >>= (noise.frequency + 1);

        int needed_cycles = 16780000 / shift_freq;

        noise_int.shift_cycles += step_cycles;

        while (noise_int.shift_cycles >= needed_cycles) {
            int carry = noise_int.shift_reg & 1;

            noise_int.shift_reg >>= 1;

            if (carry) {
                noise_int.shift_reg ^= noise.full_width ? 0x6000 : 0x60;
            }

            noise_int.output = carry;

            // consume shifting cycles
            noise_int.shift_cycles -= needed_cycles;
        }

        if (envelope.time != 0) {
            int envelope_clock = s_envelope_clock[envelope.time];

            noise_int.envelope_cycles += step_cycles;

            // TODO:
            // 1) not very optimized - i suppose
            // 2) almost the same as QUAD envelope code
            while (noise_int.envelope_cycles >= envelope_clock) {

                if (envelope.direction == ENV_INC) {
                    if (noise_int.volume != 15) noise_int.volume++;
                } else {
                    if (noise_int.volume != 0 ) noise_int.volume--;
                }

                noise_int.envelope_cycles -= envelope_clock;
            }
        }

        if (noise.apply_length) {
            noise_int.length_cycles += step_cycles;
        }
    }

    void APU::mixChannels(int samples) {
        auto& psg  = regs.control.psg;
        auto& dma  = regs.control.dma;
        auto& bias = regs.bias;

        float psg_volume = s_psg_volume[psg.volume];

        const float fifo_volume[2] = {
            APU::s_dma_volume[dma[0].volume],
            APU::s_dma_volume[dma[1].volume]
        };

        for (int sample = 0; sample < samples; sample++) {
            s16 output[2] { 0, 0 };

            float volume_left  = (float)psg.master[SIDE_LEFT ] / 7.0;
            float volume_right = (float)psg.master[SIDE_RIGHT] / 7.0;

            // generate PSG channels
            s16 tone1 = (s16)generateQuad(0);
            s16 tone2 = (s16)generateQuad(1);
            s16 wave  = (s16)generateWave();
            s16 noise = (s16)generateNoise();

            // mix PSGs on the left channel
            if (psg.enable[SIDE_LEFT][0]) output[SIDE_LEFT] += tone1;
            if (psg.enable[SIDE_LEFT][1]) output[SIDE_LEFT] += tone2;
            if (psg.enable[SIDE_LEFT][2]) output[SIDE_LEFT] += wave;
            if (psg.enable[SIDE_LEFT][3]) output[SIDE_LEFT] += noise;

            // mix PSGs on the right channel
            if (psg.enable[SIDE_RIGHT][0]) output[SIDE_RIGHT] += tone1;
            if (psg.enable[SIDE_RIGHT][1]) output[SIDE_RIGHT] += tone2;
            if (psg.enable[SIDE_RIGHT][2]) output[SIDE_RIGHT] += wave;
            if (psg.enable[SIDE_RIGHT][3]) output[SIDE_RIGHT] += noise;

            // apply master volume to the PSG channels
            output[SIDE_LEFT ] *= psg_volume * volume_left;
            output[SIDE_RIGHT] *= psg_volume * volume_right;

            // add FIFO audio samples
            for (int fifo = 0; fifo < 2; fifo++) {
                if (dma[fifo].enable[SIDE_LEFT]) {
                    output[SIDE_LEFT ] += fifo_sample[fifo] * fifo_volume[fifo];
                }
                if (dma[fifo].enable[SIDE_RIGHT]) {
                    output[SIDE_RIGHT] += fifo_sample[fifo] * fifo_volume[fifo];
                }
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

            // copy audio into the output ringbuffers
            ringbuffers[SIDE_LEFT ][write_pos] = output[SIDE_LEFT ] * 64;
            ringbuffers[SIDE_RIGHT][write_pos] = output[SIDE_RIGHT] * 64;

            // update write position
            write_pos = (write_pos + 1) & 0x3FFF;
        }

    }

    void APU::step(int step_cycles) {
        int cycles_per_sample = 16780000 / sample_rate;

        updateQuad(step_cycles);
        updateWave(step_cycles);
        updateNoise(step_cycles);

        cycles_elapsed += step_cycles;

        if (cycles_elapsed >= cycles_per_sample) {
            int samples = cycles_elapsed / cycles_per_sample;

            mixChannels(samples);

            // consume the cycles required for each sample
            cycles_elapsed -= samples * cycles_per_sample;
        }

    }

    void APU::fillBuffer(u16* stream, int length) {
        // divide length by four because:
        // 1) length is provided in bytes, while we work with hwords
        // 2) length is twice the actual length because of two stereo channels
        length >>= 2;

        for (int i = 0; i < length; i++) {
            // copy left/right sample from ringbuffer to SDL buffer
            stream[i * 2 + 0] = ringbuffers[SIDE_LEFT ][read_pos];
            stream[i * 2 + 1] = ringbuffers[SIDE_RIGHT][read_pos];

            // update read position
            read_pos = (read_pos + 1) & 0x3FFF;
        }
    }
}

/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <common/log.hpp>

#include "mp2k.hpp"

namespace nba::core {

void MP2K::SoundMainRAM(M4ASoundInfo sound_info) {
  using Access = arm::MemoryBase::Access;

  if (sound_info.magic != 0x68736D54) {
    return;
  }

  if (!engaged) {
    ASSERT(sound_info.pcmSamplesPerVBlank != 0, "MP2K: samples per V-blank must not be zero.");

    total_frame_count = kDMABufferSize / sound_info.pcmSamplesPerVBlank;
    buffer = std::make_unique<float[]>(kSamplesPerFrame * total_frame_count * 2);
    engaged = true;
  }

  auto max_channels = sound_info.maxChans;

  // TODO: clean this mess up.
  for (int i = 0; i < max_channels; i++) {
    auto& channel = sound_info.channels[i];

    u32 envelope_volume;
    u32 wav_address;

    if (channel.status & CHANNEL_ON) {
      if (!(channel.status & CHANNEL_START)) {
        envelope_volume = (u32)channel.ev;

        if (!(channel.status & CHANNEL_IEC)) {
          if (!(channel.status & CHANNEL_STOP)) {
            if ((channel.status & CHANNEL_ENV_MASK) == CHANNEL_ENV_DECAY) {
              auto envelope_sustain = channel.sustain;

              envelope_volume = (envelope_volume * channel.decay) >> 8;
            
              if (envelope_volume <= envelope_sustain) {
                if (envelope_sustain == 0) {
                  goto SetupEnvelopeEchoMaybe;
                }
                channel.status = (channel.status & ~CHANNEL_ENV_MASK) | CHANNEL_ENV_SUSTAIN;
                envelope_volume = envelope_sustain;
              }
            } else if ((channel.status & CHANNEL_ENV_MASK) == CHANNEL_ENV_ATTACK) {
              goto ApplyAttack;
            }
          } else {
            envelope_volume = (envelope_volume * channel.release) >> 8;
            if (envelope_volume <= channel.echoVolume) {
SetupEnvelopeEchoMaybe:
              if (channel.echoVolume == 0) {
                channel.status = 0;
                continue;
              }

              channel.status |= CHANNEL_IEC;
              envelope_volume = (u32)channel.echoVolume;
            }
          }
        } else {
          auto echo_length = channel.echoLength;
          auto echo_length_updated = (u32)echo_length - 1;
          channel.echoLength = echo_length_updated;
          if (echo_length == 0 || echo_length_updated == 0) {
            channel.status = 0;
            continue;
          }
        }
      } else {
        if (channel.status & CHANNEL_STOP) {
          channel.status = 0;
          continue;
        }

        channel.status = CHANNEL_ENV_ATTACK;
        // channel->current_wave_data = &wave->first_sample + channel->current_wave_time_maybe;
        // channel->current_wave_time_maybe = wave->number_of_samples - channel->current_wave_time_maybe;
        envelope_volume = 0;
        channel.ev = envelope_volume; // 0
        // Note: the following code seems to be related to looping.
        channel.fw = 0; // <- seems to go into channel.status later???
        // if ((*(byte *)((int)&wave->status + 1) & 0xc0) != 0) {
        //   channel.status = 0x13; // CHANNEL_LOOP | CHANNEL_ENV_ATTACK
        //   channel->status = '\x13';
        // }

        // TODO: especially clean THIS mess up.
        wav_address = sound_info.channels[i].wav;
        channel_cache[i].forward_loop = memory.ReadHalf(wav_address + 2, Access::Sequential) != 0;
        channel_cache[i].sample_rate = memory.ReadWord(wav_address + 4, Access::Sequential);
        channel_cache[i].loop_position = memory.ReadWord(wav_address + 8, Access::Sequential);
        channel_cache[i].number_of_samples = memory.ReadWord(wav_address + 12, Access::Sequential);
        channel_cache[i].pcm_base_address = wav_address + 16;
        channel_cache[i].current_position = 0;
        channel_cache[i].resample_phase = 0;
        channel_cache[i].should_fetch_sample = true;
        for (auto& sample : channel_cache[i].sample_history) {
          sample = 0;
        }

ApplyAttack:
        envelope_volume += channel.attack;
        if (envelope_volume > 0xFE) {
          channel.status = (channel.status & ~CHANNEL_ENV_MASK) | CHANNEL_ENV_DECAY;
          envelope_volume = 0xFF;
        }
      }
    }

    channel.ev = (u8)envelope_volume;
    envelope_volume = (envelope_volume * (sound_info.masterVolume + 1)) >> 4;
    channel.er = (u8)((envelope_volume * channel.rightVolume) >> 8);
    channel.el = (u8)((envelope_volume * channel.leftVolume) >> 8);

    // Starting here it is probably best to reference the assembly code.
    // The decompilation is really weird/confusing...
    
    // Note: the first part of this code checks if the sample should loop,
    // then calculates the pointer to the sample at the loop position and the loop length?
    // uStack00000010 = channel.status & 0x10;
    // if (uStack00000010 != 0) {
    //   in_stack_0000000c = &wave->first_sample + wave->loop_start;
    //   uStack00000010 = wave->number_of_samples - wave->loop_start;
    // }
    // current_wave_time_maybe = channel->current_wave_time_maybe;
    // current_wave_data = channel->current_wave_data;
    // channel.status = channel->fw;
  }

  this->sound_info = sound_info;
}

void MP2K::RenderFrame() {
  // TODO: move this into the class definition (same for SoundMainRAM)
  using Access = arm::MemoryBase::Access;

  // TODO: move this into a static constexpr method?
  #define S8F(value) (s8(value) / 127.0)

  // TODO: move this somewhere else?
  static constexpr float kDifferentialLUT[] = {
    S8F(0x00), S8F(0x01), S8F(0x04), S8F(0x09),
    S8F(0x10), S8F(0x19), S8F(0x24), S8F(0x31),
    S8F(0xC0), S8F(0xCF), S8F(0xDC), S8F(0xE7),
    S8F(0xF0), S8F(0xF7), S8F(0xFC), S8F(0xFF)
  };

  current_frame = (current_frame + 1) % total_frame_count;

  auto reverb = sound_info.reverb;
  auto max_channels = sound_info.maxChans;
  auto destination = &buffer[current_frame * kSamplesPerFrame * 2];

  if (reverb == 0) {
    std::memset(destination, 0, kSamplesPerFrame * 2 * sizeof(float));
  } else {
    auto factor = reverb / (128.0 * 4.0);
    auto other_frame  = (current_frame + 1) % total_frame_count;
    auto other_buffer = &buffer[other_frame * kSamplesPerFrame * 2];

    for (int i = 0; i < kSamplesPerFrame; i++) {
      float sample_out = 0;

      sample_out += other_buffer[i * 2 + 0];
      sample_out += other_buffer[i * 2 + 1];
      sample_out += destination[i * 2 + 0];
      sample_out += destination[i * 2 + 1];

      sample_out *= factor;

      destination[i * 2 + 0] = sample_out;
      destination[i * 2 + 1] = sample_out;
    }
  }
  
  for (int i = 0; i < max_channels; i++) {
    auto& channel = sound_info.channels[i];
    auto& cache = channel_cache[i];

    if ((channel.status & CHANNEL_ON) == 0 || cache.sample_rate == 0) {
      continue;
    }

    // TODO: clean the calculation of the resampling step up.
    auto sample_rate = cache.sample_rate / 1024.0;
    auto note_freq = (std::uint64_t(channel.freq) << 32) / cache.sample_rate / 16384.0;
    auto angular_step = note_freq / 256.0 * (sample_rate / float(kSampleRate));

    if ((channel.type & 8) != 0) {
      angular_step = (sound_info.pcmFreq / float(kSampleRate));
    }

    auto volume_l = channel.el / 255.0;
    auto volume_r = channel.er / 255.0;
    bool compressed = (channel.type & 32) != 0;
    auto sample_history = cache.sample_history;

    for (int j = 0; j < kSamplesPerFrame; j++) {
      if (cache.should_fetch_sample) {
        float sample;
        
        memory.the_pain = true;

        if (compressed) {
          auto block_offset  = cache.current_position & 63;
          auto block_address = cache.pcm_base_address + (cache.current_position >> 6) * 33;

          if (block_offset == 0) {
            sample = S8F(memory.ReadByte(block_address, Access::Sequential));
          } else {
            sample = sample_history[0];
          }

          auto address = block_address + (block_offset >> 1) + 1;
          auto lut_index = memory.ReadByte(address, Access::Sequential);

          if (block_offset & 1) {
            lut_index &= 15;
          } else {
            lut_index >>= 4;
          }

          sample += kDifferentialLUT[lut_index];
        } else {
          auto address = cache.pcm_base_address + cache.current_position;
          sample = S8F(memory.ReadByte(address, Access::Sequential));
        }
        
        memory.the_pain = false;
      
        sample_history[3] = sample_history[2];
        sample_history[2] = sample_history[1];
        sample_history[1] = sample_history[0];
        sample_history[0] = sample;

        cache.should_fetch_sample = false;
      }

      // http://paulbourke.net/miscellaneous/interpolation/
      float mu  = cache.resample_phase;
      // float mu2 = mu * mu;
      // float a0 = sample_history[0] - sample_history[1] - sample_history[3] + sample_history[2];
      // float a1 = sample_history[3] - sample_history[2] - a0;
      // float a2 = sample_history[1] - sample_history[3];
      // float a3 = sample_history[2]; 
      // float sample = a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3;
      float sample = sample_history[0] * mu + sample_history[1] * (1.0 - mu);

      destination[j * 2 + 0] += sample * volume_r;
      destination[j * 2 + 1] += sample * volume_l;

      cache.resample_phase += angular_step;

      if (cache.resample_phase >= 1) {
        auto n = int(cache.resample_phase);
        cache.resample_phase -= n;
        cache.current_position += n;
        cache.should_fetch_sample = true;

        if (cache.current_position >= cache.number_of_samples) {
          if (cache.forward_loop) {
            cache.current_position = cache.loop_position + n - 1;
          } else {
            cache.current_position = cache.number_of_samples;
            cache.should_fetch_sample = false;
          }
        }
      }
    }
  }
}

auto MP2K::ReadSample() -> float* {
  if (buffer_read_index == 0) {
    RenderFrame();
  }

  auto sample = &buffer[(current_frame * kSamplesPerFrame + buffer_read_index) * 2];

  if (++buffer_read_index == kSamplesPerFrame) {
    buffer_read_index = 0;
  }

  return sample;
}

} // namespace nba::core

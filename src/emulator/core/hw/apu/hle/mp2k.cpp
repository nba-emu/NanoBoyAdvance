/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <common/log.hpp>
#include <emulator/core/cpu.hpp>

#include "mp2k.hpp"

namespace nba::core {

void MP2K::Reset() {
  engaged = false;
  use_cubic_filter = false;
  total_frame_count = 0;
  current_frame = 0;
  buffer_read_index = 0;
  for (auto& entry : samplers) {
    entry = {};
  }
}

void MP2K::SoundMainRAM(SoundInfo const& sound_info) {
  if (sound_info.magic != 0x68736D54) {
    return;
  }

  if (!engaged) {
    ASSERT(sound_info.pcm_samples_per_vblank != 0, "MP2K: samples per V-blank must not be zero.");

    total_frame_count = kDMABufferSize / sound_info.pcm_samples_per_vblank;
    buffer = std::make_unique<float[]>(kSamplesPerFrame * total_frame_count * 2);
    engaged = true;
  }

  auto max_channels = std::min(sound_info.max_channels, kMaxSoundChannels);

  this->sound_info = sound_info;

  for (int i = 0; i < max_channels; i++) {
    auto& channel = this->sound_info.channels[i];
    auto& sampler = samplers[i];
    auto  envelope_volume = u32(channel.envelope_volume);
    auto  envelope_phase = channel.status & CHANNEL_ENV_MASK;

    if ((channel.status & CHANNEL_ON) == 0) {
      continue;
    }

    if (channel.status & CHANNEL_START) {
      if (channel.status & CHANNEL_STOP) {
        channel.status = 0;
        continue;
      }

      envelope_volume = channel.envelope_attack;
      if (envelope_volume == 0xFF) {
        channel.status = CHANNEL_ENV_DECAY;
      } else {
        channel.status = CHANNEL_ENV_ATTACK;
      }

      if (cpu.Read<u8, true>(channel.wave_data_address + 3) & 0xC0) {
        channel.status |= CHANNEL_LOOP;
      }

      sampler = {};
      sampler.wave.pcm_base_address = channel.wave_data_address + 16;
      sampler.wave.number_of_samples = cpu.Read<u32, true>(channel.wave_data_address + 12);
      sampler.wave.loop_position = cpu.Read<u32, true>(channel.wave_data_address + 8);
    } else if (channel.status & CHANNEL_ECHO) {
      if (channel.echo_length-- == 0) {
        channel.status = 0;
        continue;
      }
    } else if (channel.status & CHANNEL_STOP) {
      envelope_volume = (envelope_volume * channel.envelope_release) >> 8;

      if (envelope_volume <= channel.echo_volume) {
        if (channel.echo_volume == 0) {
          channel.status = 0;
          continue;
        }

        channel.status |= CHANNEL_ECHO;
        envelope_volume = (u32)channel.echo_volume;
      }
    } else if (envelope_phase == CHANNEL_ENV_ATTACK) {
      envelope_volume += channel.envelope_attack;

      if (envelope_volume > 0xFE) {
        channel.status = (channel.status & ~CHANNEL_ENV_MASK) | CHANNEL_ENV_DECAY;
        envelope_volume = 0xFF;
      }
    } else if (envelope_phase == CHANNEL_ENV_DECAY) {
      envelope_volume = (envelope_volume * channel.envelope_decay) >> 8;
    
      auto envelope_sustain = channel.envelope_sustain;
      if (envelope_volume <= envelope_sustain) {
        if (envelope_sustain == 0 && channel.echo_volume == 0) {
          channel.status = 0;
          continue;
        }

        channel.status = (channel.status & ~CHANNEL_ENV_MASK) | CHANNEL_ENV_SUSTAIN;
        envelope_volume = envelope_sustain;
      }
    } 

    channel.envelope_volume = u8(envelope_volume);
    envelope_volume = (envelope_volume * (this->sound_info.master_volume + 1)) >> 4;
    channel.envelope_volume_r = u8((envelope_volume * channel.volume_r) >> 8);
    channel.envelope_volume_l = u8((envelope_volume * channel.volume_l) >> 8);
  }
}

void MP2K::RenderFrame() {
  static constexpr float kDifferentialLUT[] = {
    S8ToFloat(0x00), S8ToFloat(0x01), S8ToFloat(0x04), S8ToFloat(0x09),
    S8ToFloat(0x10), S8ToFloat(0x19), S8ToFloat(0x24), S8ToFloat(0x31),
    S8ToFloat(0xC0), S8ToFloat(0xCF), S8ToFloat(0xDC), S8ToFloat(0xE7),
    S8ToFloat(0xF0), S8ToFloat(0xF7), S8ToFloat(0xFC), S8ToFloat(0xFF)
  };

  current_frame = (current_frame + 1) % total_frame_count;

  auto reverb = sound_info.reverb;
  auto max_channels = std::min(sound_info.max_channels, kMaxSoundChannels);
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
    auto& sampler = samplers[i];

    if ((channel.status & CHANNEL_ON) == 0) {
      continue;
    }

    float angular_step;

    if (channel.type & 8) {
      angular_step = sound_info.pcm_sample_rate / float(kSampleRate);
    } else {
      angular_step = channel.frequency / float(kSampleRate);
    }

    auto volume_l = channel.envelope_volume_l / 255.0;
    auto volume_r = channel.envelope_volume_r / 255.0;
    bool compressed = (channel.type & 32) != 0;
    auto sample_history = sampler.sample_history;

    for (int j = 0; j < kSamplesPerFrame; j++) {
      if (sampler.should_fetch_sample) {
        float sample;

        if (compressed) {
          auto block_offset  = sampler.current_position & 63;
          auto block_address = sampler.wave.pcm_base_address + (sampler.current_position >> 6) * 33;

          if (block_offset == 0) {
            sample = S8ToFloat(cpu.Read<u8, true>(block_address));
          } else {
            sample = sample_history[0];
          }

          auto address = block_address + (block_offset >> 1) + 1;
          auto lut_index = cpu.Read<u8, true>(address);

          if (block_offset & 1) {
            lut_index &= 15;
          } else {
            lut_index >>= 4;
          }

          sample += kDifferentialLUT[lut_index];
        } else {
          auto address = sampler.wave.pcm_base_address + sampler.current_position;
          sample = S8ToFloat(cpu.Read<u8, true>(address));
        }

        if (UseCubicFilter()) {
          sample_history[3] = sample_history[2];
          sample_history[2] = sample_history[1];
        }
        sample_history[1] = sample_history[0];
        sample_history[0] = sample;

        sampler.should_fetch_sample = false;
      }

      float sample;
      float mu = sampler.resample_phase;

      if (UseCubicFilter()) {
        // http://paulbourke.net/miscellaneous/interpolation/
        float mu2 = mu * mu;
        float a0 = sample_history[0] - sample_history[1] - sample_history[3] + sample_history[2];
        float a1 = sample_history[3] - sample_history[2] - a0;
        float a2 = sample_history[1] - sample_history[3];
        float a3 = sample_history[2]; 
        sample = a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3;
      } else {
        sample = sample_history[0] * mu + sample_history[1] * (1.0 - mu);
      }

      destination[j * 2 + 0] += sample * volume_r;
      destination[j * 2 + 1] += sample * volume_l;

      sampler.resample_phase += angular_step;

      if (sampler.resample_phase >= 1) {
        auto n = int(sampler.resample_phase);
        sampler.resample_phase -= n;
        sampler.current_position += n;
        sampler.should_fetch_sample = true;

        if (sampler.current_position >= sampler.wave.number_of_samples) {
          if (channel.status & CHANNEL_LOOP) {
            sampler.current_position = sampler.wave.loop_position + n - 1;
          } else {
            sampler.current_position = sampler.wave.number_of_samples;
            sampler.should_fetch_sample = false;
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

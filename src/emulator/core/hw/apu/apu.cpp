/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cmath>
#include <common/dsp/resampler/blep.hpp>
#include <common/dsp/resampler/cosine.hpp>
#include <common/dsp/resampler/cubic.hpp>
#include <common/dsp/resampler/nearest.hpp>
#include <common/dsp/resampler/windowed-sinc.hpp>

#include "apu.hpp"

namespace nba::core {

// See callback.cpp for implementation
void AudioCallback(APU* apu, s16* stream, int byte_len);

APU::APU(
  Scheduler& scheduler,
  DMA& dma,
  arm::MemoryBase& memory,
  std::shared_ptr<Config> config
)   : mmio(scheduler)
    , scheduler(scheduler)
    , dma(dma)
    , memory(memory)
    , config(config) {
}

void APU::Reset() {
  using namespace common::dsp;

  mmio.fifo[0].Reset();
  mmio.fifo[1].Reset();
  mmio.psg1.Reset();
  mmio.psg2.Reset();
  mmio.psg3.Reset();
  mmio.psg4.Reset();
  mmio.soundcnt.Reset();
  mmio.bias.Reset();

  resolution_old = 0;
  scheduler.Add(mmio.bias.GetSampleInterval(), this, &APU::StepMixer);
  scheduler.Add(BaseChannel::s_cycles_per_step, this, &APU::StepSequencer);

  mp2k = {};

  auto audio_dev = config->audio_dev;
  audio_dev->Close();
  audio_dev->Open(this, (AudioDevice::Callback)AudioCallback);

  using Interpolation = Config::Audio::Interpolation;

  buffer = std::make_shared<common::dsp::StereoRingBuffer<float>>(audio_dev->GetBlockSize() * 4, true);

  switch (config->audio.interpolation) {
    case Interpolation::Cosine:
      resampler = std::make_unique<CosineStereoResampler<float>>(buffer);
      break;
    case Interpolation::Cubic:
      resampler = std::make_unique<CubicStereoResampler<float>>(buffer);
      break;
    case Interpolation::Sinc_32:
      resampler = std::make_unique<SincStereoResampler<float, 32>>(buffer);
      break;
    case Interpolation::Sinc_64:
      resampler = std::make_unique<SincStereoResampler<float, 64>>(buffer);
      break;
    case Interpolation::Sinc_128:
      resampler = std::make_unique<SincStereoResampler<float, 128>>(buffer);
      break;
    case Interpolation::Sinc_256:
      resampler = std::make_unique<SincStereoResampler<float, 256>>(buffer);
      break;
  }

  if (config->audio.interpolate_fifo) {
    for (int fifo = 0; fifo < 2; fifo++) {
      fifo_buffer[fifo] = std::make_shared<RingBuffer<float>>(16, true);
      fifo_resampler[fifo] = std::make_unique<BlepResampler<float>>(fifo_buffer[fifo]);
      fifo_samplerate[fifo] = 0;
    }
  }

  resampler->SetSampleRates(mmio.bias.GetSampleRate(), audio_dev->GetSampleRate());
}

void APU::OnTimerOverflow(int timer_id, int times, int samplerate) {
  auto const& soundcnt = mmio.soundcnt;

  if (!soundcnt.master_enable) {
    return;
  }

  constexpr DMA::Occasion occasion[2] = { DMA::Occasion::FIFO0, DMA::Occasion::FIFO1 };

  for (int fifo_id = 0; fifo_id < 2; fifo_id++) {
    if (soundcnt.dma[fifo_id].timer_id == timer_id) {
      auto& fifo = mmio.fifo[fifo_id];
      for (int time = 0; time < times - 1; time++) {
        fifo.Read();
      }
      if (config->audio.interpolate_fifo) {
        if (samplerate != fifo_samplerate[fifo_id]) {
          fifo_resampler[fifo_id]->SetSampleRates(samplerate, mmio.bias.GetSampleRate());
          fifo_samplerate[fifo_id] = samplerate;
        }
        fifo_resampler[fifo_id]->Write(fifo.Read() / 128.0);
      } else {
        latch[fifo_id] = fifo.Read();
      }
      if (fifo.Count() <= 16) {
        dma.Request(occasion[fifo_id]);
      }
    }
  }
}

void APU::StepMixer(int cycles_late) {
  constexpr int psg_volume_tab[4] = { 1, 2, 4, 0 };
  constexpr int dma_volume_tab[2] = { 2, 4 };

  auto& psg = mmio.soundcnt.psg;
  auto& dma = mmio.soundcnt.dma;

  auto psg_volume = psg_volume_tab[psg.volume];

  if (mp2k.engaged) {
    common::dsp::StereoSample<float> sample { 0, 0 };

    if (resolution_old != 1) {
      resampler->SetSampleRates(65536, config->audio_dev->GetSampleRate());
      resolution_old = 1;
    }

    float* mp2k_sample = &mp2k.buffer_[(mp2k.current_frame * MP2K::kSamplesPerFrame + mp2k.read_index) * 2];

    if (mp2k.read_index == 0) {
      MP2KCustomMixer();
    }

    for (int channel = 0; channel < 2; channel++) {
      s16 psg_sample = 0;

      if (psg.enable[channel][0]) psg_sample += mmio.psg1.GetSample();
      if (psg.enable[channel][1]) psg_sample += mmio.psg2.GetSample();
      if (psg.enable[channel][2]) psg_sample += mmio.psg3.GetSample();
      if (psg.enable[channel][3]) psg_sample += mmio.psg4.GetSample();

      sample[channel] += psg_sample * psg_volume * psg.master[channel] / (28.0 * 0x200);

      /* TODO: we assume that MP2K sends right channel to FIFO A and left channel to FIFO B,
       * but we haven't verified that this is actually correct.
       */
      for (int fifo = 0; fifo < 2; fifo++) {
        if (dma[fifo].enable[channel]) {
          sample[channel] += mp2k_sample[fifo] * dma_volume_tab[dma[fifo].volume] * 0.25;
        }
      }
    }

    if (++mp2k.read_index == MP2K::kSamplesPerFrame) {
      mp2k.read_index = 0;
      mp2k.current_frame = (mp2k.current_frame + 1) % mp2k.total_frame_count;
    }

    buffer_mutex.lock();
    resampler->Write(sample);
    buffer_mutex.unlock();

    scheduler.Add(256 - (scheduler.GetTimestampNow() & 255), this, &APU::StepMixer);
  } else {
    common::dsp::StereoSample<s16> sample { 0, 0 };

    auto& bias = mmio.bias;

    if (bias.resolution != resolution_old) {
      resampler->SetSampleRates(bias.GetSampleRate(),
        config->audio_dev->GetSampleRate());
      resolution_old = mmio.bias.resolution;
      if (config->audio.interpolate_fifo) {
        for (int fifo = 0; fifo < 2; fifo++) {
          fifo_resampler[fifo]->SetSampleRates(fifo_samplerate[fifo], mmio.bias.GetSampleRate());
        }
      }
    }

    if (config->audio.interpolate_fifo) {
      for (int fifo = 0; fifo < 2; fifo++) {
        latch[fifo] = s8(fifo_buffer[fifo]->Read() * 127.0);
      }
    }

    for (int channel = 0; channel < 2; channel++) {
      s16 psg_sample = 0;

      if (psg.enable[channel][0]) psg_sample += mmio.psg1.GetSample();
      if (psg.enable[channel][1]) psg_sample += mmio.psg2.GetSample();
      if (psg.enable[channel][2]) psg_sample += mmio.psg3.GetSample();
      if (psg.enable[channel][3]) psg_sample += mmio.psg4.GetSample();

      sample[channel] += psg_sample * psg_volume * psg.master[channel] / 28;

      for (int fifo = 0; fifo < 2; fifo++) {
        if (dma[fifo].enable[channel]) {
          sample[channel] += latch[fifo] * dma_volume_tab[dma[fifo].volume];
        }
      }

      sample[channel] += mmio.bias.level;
      sample[channel]  = std::clamp(sample[channel], s16(0), s16(0x3FF));
      sample[channel] -= 0x200;
    }

    buffer_mutex.lock();
    resampler->Write({ sample[0] / float(0x200), sample[1] / float(0x200) });
    buffer_mutex.unlock();

    scheduler.Add(mmio.bias.GetSampleInterval() - cycles_late, this, &APU::StepMixer);
  }
}

void APU::StepSequencer(int cycles_late) {
  mmio.psg1.Tick();
  mmio.psg2.Tick();
  mmio.psg3.Tick();
  mmio.psg4.Tick();

  scheduler.Add(BaseChannel::s_cycles_per_step - cycles_late, this, &APU::StepSequencer);
}

enum Status : u8 {
  CHANNEL_START = 0x80,
  CHANNEL_STOP = 0x40,
  CHANNEL_LOOP = 0x10,
  CHANNEL_IEC = 0x04,

  CHANNEL_ENV_MASK = 0x03,
  CHANNEL_ENV_ATTACK = 0x03,
  CHANNEL_ENV_DECAY = 0x02,
  CHANNEL_ENV_SUSTAIN = 0x01,
  CHANNEL_ENV_RELEASE = 0x00,
  
  CHANNEL_ON = CHANNEL_START | CHANNEL_STOP | CHANNEL_IEC | CHANNEL_ENV_MASK 
};

// TODO: move this to the member variables, nerd.
static struct {
  bool forward_loop;
  u32 sample_rate;
  u32 loop_position;
  u32 number_of_samples;
  u32 pcm_base_address;
  int current_position;

  float sample_history[4];
  float resample_phase;
  bool should_fetch_sample;
} channel_cache[kM4AMaxDirectSoundChannels];

void APU::MP2KSoundMainRAM(M4ASoundInfo sound_info) {
  using Access = arm::MemoryBase::Access;

  if (sound_info.magic != 0x68736D54) {
    return;
  }

  if (!mp2k.engaged) {
    ASSERT(sound_info.pcmSamplesPerVBlank != 0, "MP2K: samples per V-blank must not be zero.");

    mp2k.total_frame_count = MP2K::kDMABufferSize / sound_info.pcmSamplesPerVBlank;
    mp2k.buffer_ = std::make_unique<float[]>(
      MP2K::kSamplesPerFrame * mp2k.total_frame_count * 2
    );
    mp2k.engaged = true;
  }

  auto max_channels = sound_info.maxChans;

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

        // custom zorgz
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

  mp2k.sound_info = sound_info;
}

void APU::MP2KCustomMixer() {
  using Access = arm::MemoryBase::Access;

  #define S8F(value) (s8(value) / 127.0)

  static constexpr float kDifferentialLUT[] = {
    S8F(0x00), S8F(0x01), S8F(0x04), S8F(0x09),
    S8F(0x10), S8F(0x19), S8F(0x24), S8F(0x31),
    S8F(0xC0), S8F(0xCF), S8F(0xDC), S8F(0xE7),
    S8F(0xF0), S8F(0xF7), S8F(0xFC), S8F(0xFF)
  };

  auto const& sound_info = mp2k.sound_info;
  auto reverb = sound_info.reverb;
  auto max_channels = sound_info.maxChans;

  auto destination = &mp2k.buffer_[mp2k.current_frame * MP2K::kSamplesPerFrame * 2];

  if (reverb == 0) {
    std::memset(destination, 0, MP2K::kSamplesPerFrame * 2 * sizeof(float));
  } else {
    auto factor = reverb / (128.0 * 4.0);
    auto other_frame  = (mp2k.current_frame + 1) % mp2k.total_frame_count;
    auto other_buffer = &mp2k.buffer_[other_frame * MP2K::kSamplesPerFrame * 2];

    for (int i = 0; i < MP2K::kSamplesPerFrame; i++) {
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
    auto angular_step = note_freq / 256.0 * (sample_rate / float(MP2K::kSampleRate));

    if ((channel.type & 8) != 0) {
      angular_step = (sound_info.pcmFreq / float(MP2K::kSampleRate));
    }

    auto volume_l = channel.el / 255.0;
    auto volume_r = channel.er / 255.0;
    bool compressed = (channel.type & 32) != 0;
    auto sample_history = cache.sample_history;

    for (int j = 0; j < MP2K::kSamplesPerFrame; j++) {
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
      float mu2 = mu * mu;
      float a0 = sample_history[0] - sample_history[1] - sample_history[3] + sample_history[2];
      float a1 = sample_history[3] - sample_history[2] - a0;
      float a2 = sample_history[1] - sample_history[3];
      float a3 = sample_history[2]; 
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

} // namespace nba::core

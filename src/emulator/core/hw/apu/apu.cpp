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

// FIXME
#include <fstream>
static std::ofstream dump{"audio_out.bin", std::ios::binary};

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
  // TODO: unify as much code as possible
  // TODO: remove the interpolate FIFOs option? it might be a maintanence burden...

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

    float* mp2k_sample = mp2k.buffer[mp2k.read_index];

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

      /* TODO: we assume that MP2K sends left channel to FIFO A and right channel to FIFO B,
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

// TODO: move this to the member variables, nerd.
static struct {
  bool forward_loop;
  std::uint32_t frequency;
  std::uint32_t loop_sample_index;
  std::uint32_t number_of_samples;
  std::uint32_t data_address;
  float current_sample_index;
} channel_cache[kM4AMaxDirectSoundChannels];

void APU::MP2KSoundMainRAM(M4ASoundInfo sound_info) {
  using Access = arm::MemoryBase::Access;

  // https://github.com/pret/pokeemerald/blob/9a195c0fef7a47e5ab55c7047d80969c9cd6e44e/include/gba/m4a_internal.h#L70-L79
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

  auto max_channels = sound_info.maxChans;

  // TODO: perform some validation (magic number e.g.) before claiming to be enganged.
  mp2k.engaged = true;

  for (int i = 0; i < max_channels; i++) {
    auto& channel = sound_info.channels[i];

    auto channel_status = channel.status;
    auto channel_status_updated = (u32)channel_status;
    u32 envelope_volume;
    u32 wav_address;

    // TODO: make sense of this hot mess and clean it up...
    if (channel_status & CHANNEL_ON) {
      if (!(channel_status & CHANNEL_START)) {
        envelope_volume = (u32)channel.ev;

        if (!(channel_status & CHANNEL_IEC)) {
          if (!(channel_status & CHANNEL_STOP)) {
            if ((channel_status_updated & CHANNEL_ENV_MASK) == CHANNEL_ENV_DECAY) {
              auto envelope_sustain = channel.sustain;

              envelope_volume = (envelope_volume * channel.decay) >> 8;
            
              if (envelope_volume <= envelope_sustain) {
                if (envelope_sustain == 0) {
                  goto SetupEnvelopeEchoMaybe;
                }

                channel_status_updated--; // -> CHANNEL_ENV_SUSTAIN
                channel.status = (u8)channel_status_updated;
                envelope_volume = envelope_sustain;
              }
            } else if ((channel_status_updated & CHANNEL_ENV_MASK) == CHANNEL_ENV_ATTACK) {
              goto LAB_082df230;
            }
          } else {
            envelope_volume = (envelope_volume * channel.release) >> 8;
            if (envelope_volume <= channel.echoVolume) {
SetupEnvelopeEchoMaybe:
              if (channel.echoVolume == 0) {
                goto StopChannel;
              }

              channel_status_updated = channel_status_updated | CHANNEL_IEC;
              channel.status = (u8)channel_status_updated;
              envelope_volume = (u32)channel.echoVolume;
            }
          }
        
        } else {
          auto echo_length = channel.echoLength;
          auto echo_length_updated = (u32)echo_length - 1;
          channel.echoLength = echo_length_updated;
          if (echo_length == 0 || echo_length_updated == 0) {
            goto StopChannel;
          }
        }
      } else {
        if (channel_status & CHANNEL_STOP) {
StopChannel:
          channel.status = 0;
          continue;
        }

        channel_status_updated = CHANNEL_ENV_ATTACK;
        channel.status = channel_status_updated;
        // channel->current_wave_data = &wave->first_sample + channel->current_wave_time_maybe;
        // channel->current_wave_time_maybe = wave->number_of_samples - channel->current_wave_time_maybe;
        envelope_volume = 0;
        channel.ev = envelope_volume; // 0
        // Note: the following code seems to be related to looping.
        channel.fw = 0; // <- seems to go into channel_status_updated later???
        // if ((*(byte *)((int)&wave->status + 1) & 0xc0) != 0) {
        //   channel_status_updated = 0x13; // CHANNEL_LOOP | CHANNEL_ENV_ATTACK
        //   channel->status = '\x13';
        // }

        // custom zorgz
        wav_address = sound_info.channels[i].wav;
        channel_cache[i].forward_loop = memory.ReadHalf(wav_address + 2, Access::Sequential) != 0;
        channel_cache[i].frequency = memory.ReadWord(wav_address + 4, Access::Sequential);
        channel_cache[i].loop_sample_index = memory.ReadWord(wav_address + 8, Access::Sequential);
        channel_cache[i].number_of_samples = memory.ReadWord(wav_address + 12, Access::Sequential);
        channel_cache[i].data_address = wav_address + 16;
        channel_cache[i].current_sample_index = 0;

LAB_082df230:
        envelope_volume += channel.attack;
        if (envelope_volume > 0xFE) {
          channel_status_updated--;
          channel.status = channel_status_updated;
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
    // uStack00000010 = channel_status_updated & 0x10;
    // if (uStack00000010 != 0) {
    //   in_stack_0000000c = &wave->first_sample + wave->loop_start;
    //   uStack00000010 = wave->number_of_samples - wave->loop_start;
    // }
    // current_wave_time_maybe = channel->current_wave_time_maybe;
    // current_wave_data = channel->current_wave_data;
    // channel_status_updated = channel->fw;

    //fmt::print("{:02X} ", channel.ev);
  }

  mp2k.sound_info = sound_info;
}

void APU::MP2KCustomMixer() {
  using Access = arm::MemoryBase::Access;

  auto const& sound_info = mp2k.sound_info;
  auto reverb = sound_info.reverb;
  auto max_channels = sound_info.maxChans;

  // TODO: reset buffer read index to avoid desync?
  // TODO: round up number of samples instead of rounding down?

  // mp2k.read_index = 0;

  if (reverb == 0) {
    std::memset(mp2k.buffer, 0, sizeof(mp2k.buffer));
  } else {
    // Reverb algorithm as found in Pokémon:
    for (int i = 0; i < MP2K::kSamplesPerFrame; i++) {
      auto new_sample = (mp2k.buffer[i][0] * reverb + mp2k.buffer[i][1]) / 256.0;
      mp2k.buffer[i][0] = new_sample;
      mp2k.buffer[i][1] = new_sample;
    }
  }
  
  for (int i = 0; i < max_channels; i++) {
    auto& channel = sound_info.channels[i];

    // Channel is not playing I guess?
    if ((channel.status & 0xC7) == 0) {
      continue;
    }

    auto& cache = channel_cache[i];

    // if (cache.frequency == 0) {
    //   // Welp, not sure what to do in that case.
    //   continue;
    // }

    auto sample_rate = cache.frequency / 1024.0;
    auto note_freq = (std::uint64_t(channel.freq) << 32) / cache.frequency / 16384.0;
    auto angular_step = note_freq / 256.0 * (sample_rate / 65536.0);

    // TODO: clean this big mess up.
    if (channel.type == 1) continue;
    if (channel.type == 2) continue;
    if (channel.type == 3) continue;
    if (channel.type == 4) continue;
    if (channel.type == 8) {
      // TODO: this appears to be wrong?
      angular_step = (sound_info.pcmFreq / 65536.0);
      // angular_step = sample_rate / 65536.0;
    }

    for (int j = 0; j < MP2K::kSamplesPerFrame; j++) {
      // TODO: implement cubic interpolation in a cleaner, more accurate way!!!
      auto index0 = int(cache.current_sample_index);
      auto index1 = std::max(0, index0 - 1);
      auto index2 = std::max(0, index0 - 2);
      auto index3 = std::max(0, index0 - 3);

      memory.the_pain = true;

      auto sample3 = std::int8_t(memory.ReadByte(cache.data_address + index3, Access::Sequential)) / 128.0;
      auto sample2 = std::int8_t(memory.ReadByte(cache.data_address + index2, Access::Sequential)) / 128.0;
      auto sample1 = std::int8_t(memory.ReadByte(cache.data_address + index1, Access::Sequential)) / 128.0;
      auto sample0 = std::int8_t(memory.ReadByte(cache.data_address + index0, Access::Sequential)) / 128.0;

      memory.the_pain = false;

      float a0, a1, a2, a3;
      float mu, mu2;    
      mu  = cache.current_sample_index - int(cache.current_sample_index);
      mu2 = mu * mu;
      a0 = sample0 - sample1 - sample3 + sample2;
      a1 = sample3 - sample2 - a0;
      a2 = sample1 - sample3;
      a3 = sample2;

      auto sample = a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3;

      mp2k.buffer[j][0] += sample * channel.el / 255.0;
      mp2k.buffer[j][1] += sample * channel.er / 255.0;

      cache.current_sample_index += angular_step;

      if (cache.current_sample_index >= cache.number_of_samples) {
        if (cache.forward_loop) {
          do {
            cache.current_sample_index -= cache.number_of_samples;
          } while (cache.current_sample_index >= cache.number_of_samples);

          cache.current_sample_index += cache.loop_sample_index;
        } else {
          cache.current_sample_index = cache.number_of_samples;
        }
      }
    }
  }

  // for (int i = 0; i < MP2K::kSamplesPerFrame; i++) {
  //   mp2k.derp_buffer->Write({mp2k.buffer[i][0], mp2k.buffer[i][1]});
  // }

  // Dump samples to disk for debugging
  dump.write((char*)mp2k.buffer, sizeof(mp2k.buffer));
}

} // namespace nba::core

/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cmath>
#include <nba/common/dsp/resampler/blep.hpp>
#include <nba/common/dsp/resampler/cosine.hpp>
#include <nba/common/dsp/resampler/cubic.hpp>
#include <nba/common/dsp/resampler/nearest.hpp>
#include <nba/common/dsp/resampler/sinc.hpp>

#include "apu.hpp"

namespace nba::core {

// See callback.cpp for implementation
void AudioCallback(APU* apu, s16* stream, int byte_len);

APU::APU(
  Scheduler& scheduler,
  DMA& dma,
  Bus& bus,
  std::shared_ptr<Config> config
)   : mmio(scheduler)
    , scheduler(scheduler)
    , dma(dma)
    , mp2k(bus)
    , config(config) {
}

APU::~APU() {
  config->audio_dev->Close();
}

void APU::Reset() {
  using Interpolation = Config::Audio::Interpolation;

  mmio.fifo[0].Reset();
  mmio.fifo[1].Reset();
  mmio.psg1.Reset();
  mmio.psg2.Reset();
  mmio.psg3.Reset(WaveChannel::ResetWaveRAM::Yes);
  mmio.psg4.Reset();
  mmio.soundcnt.Reset();
  mmio.bias.Reset();
  fifo_pipe[0] = {};
  fifo_pipe[1] = {};

  resolution_old = 0;
  scheduler.Add(mmio.bias.GetSampleInterval(), this, &APU::StepMixer);
  scheduler.Add(BaseChannel::s_cycles_per_step, this, &APU::StepSequencer);

  mp2k.Reset();
  mp2k_read_index = {};

  auto audio_dev = config->audio_dev;
  audio_dev->Close();
  audio_dev->Open(this, (AudioDevice::Callback)AudioCallback);

  buffer = std::make_shared<StereoRingBuffer<float>>(audio_dev->GetBlockSize() * 4, true);

  switch (config->audio.interpolation) {
    case Interpolation::Cosine:   resampler = std::make_unique<CosineStereoResampler<float>>(buffer); break;
    case Interpolation::Cubic:    resampler = std::make_unique<CubicStereoResampler<float>>(buffer);  break;
    case Interpolation::Sinc_32:  resampler = std::make_unique<SincStereoResampler<float, 32>>(buffer);  break;
    case Interpolation::Sinc_64:  resampler = std::make_unique<SincStereoResampler<float, 64>>(buffer);  break;
    case Interpolation::Sinc_128: resampler = std::make_unique<SincStereoResampler<float, 128>>(buffer); break;
    case Interpolation::Sinc_256: resampler = std::make_unique<SincStereoResampler<float, 256>>(buffer); break;
  }

  // @todo: remove this feature, as it is not used (exposed) anymore.
  if (config->audio.interpolate_fifo) {
    for (int fifo = 0; fifo < 2; fifo++) {
      fifo_buffer[fifo] = std::make_shared<RingBuffer<float>>(16, true);
      fifo_resampler[fifo] = std::make_unique<BlepResampler<float>>(fifo_buffer[fifo]);
      fifo_samplerate[fifo] = 0;
    }
  }

  resampler->SetSampleRates(mmio.bias.GetSampleRate(), audio_dev->GetSampleRate());

  // bleh, stupid std::atomic getting in the way
  // drc = {};
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
      auto& pipe = fifo_pipe[fifo_id];

      if (fifo.Count() <= 3) {
        dma.Request(occasion[fifo_id]);
      }

      if (pipe.size == 0 && fifo.Count() > 0) {
        pipe.word = fifo.ReadWord();
        pipe.size = 4;
      }

      s8 sample = (s8)(u8)pipe.word;

      if (pipe.size > 0) {
        pipe.word >>= 8;
        pipe.size--;
      }

      if (config->audio.interpolate_fifo) {
        if (samplerate != fifo_samplerate[fifo_id]) {
          fifo_resampler[fifo_id]->SetSampleRates(samplerate, mmio.bias.GetSampleRate());
          fifo_samplerate[fifo_id] = samplerate;
        }
        fifo_resampler[fifo_id]->Write(sample / 128.0);
      } else {
        latch[fifo_id] = sample;
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

  if (mp2k.IsEngaged()) {
    StereoSample<float> sample { 0, 0 };

    if (resolution_old != 1) {
      resampler->SetSampleRates(65536, config->audio_dev->GetSampleRate());
      resolution_old = 1;
    }

    auto mp2k_sample = mp2k.ReadSample();

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

    if(!mmio.soundcnt.master_enable) sample = {};

    buffer_mutex.lock();
    resampler->Write(sample);
    buffer_mutex.unlock();

    scheduler.Add(256 - (scheduler.GetTimestampNow() & 255), this, &APU::StepMixer);
  } else {
    StereoSample<s16> sample { 0, 0 };

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

    if(!mmio.soundcnt.master_enable) sample = {};

    buffer_mutex.lock();
    resampler->Write({ sample[0] / float(0x200), sample[1] / float(0x200) });
    buffer_mutex.unlock();

    // @todo: force-align this to the system clock.
    scheduler.Add(mmio.bias.GetSampleInterval() - cycles_late, this, &APU::StepMixer);
  }

  drc.samples_written++;

  // @todo: this could be hooked to the PPU 
  u64 timestamp_now = scheduler.GetTimestampNow();
  u64 diff = timestamp_now - drc.timestamp_last_update;

  if(diff >= 280896*60) {
    auto time_point_now = std::chrono::steady_clock::now();

    auto time_delta = std::chrono::duration_cast<std::chrono::microseconds>(
      time_point_now - drc.time_point_last_update).count();

    float guest_sample_rate = drc.samples_written / (float)time_delta * 1000000;

    // @todo: do this in a non-shitty way
    guest_sample_rate *= (float)config->audio_dev->GetSampleRate() / (mp2k.IsEngaged() ? 65536 : mmio.bias.GetSampleRate());

    // fmt::print("guest: {} Hz host: {} Hz\n", guest_sample_rate, drc.host_sample_rate);

    if(guest_sample_rate > 100 && drc.host_sample_rate > 0) {
      const float kDecay = 0.35;
      const float kTolerance = 0.0075; // @todo: figure out what a good value is.
      const float kMin = 1 - kTolerance;
      const float kMax = 1 + kTolerance;

      float new_scale = guest_sample_rate / drc.host_sample_rate;
      float smoothed_scale = std::clamp((new_scale - drc.old_scale) * kDecay + drc.old_scale, kMin, kMax);

      resampler->SetRateScale(smoothed_scale);
      fmt::print("rate: {}\n", smoothed_scale);

      drc.old_scale = smoothed_scale;
    }

    drc.samples_written = 0;
    drc.time_point_last_update = time_point_now;
    drc.timestamp_last_update = timestamp_now;
  }
}

void APU::StepSequencer(int cycles_late) {
  mmio.psg1.Tick();
  mmio.psg2.Tick();
  mmio.psg3.Tick();
  mmio.psg4.Tick();

  scheduler.Add(BaseChannel::s_cycles_per_step - cycles_late, this, &APU::StepSequencer);
}

} // namespace nba::core

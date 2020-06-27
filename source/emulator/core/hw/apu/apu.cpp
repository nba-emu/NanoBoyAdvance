/*
 * Copyright (C) 2020 fleroviux
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

/* Implemented in callback.cpp */
void AudioCallback(APU* apu, std::int16_t* stream, int byte_len);

APU::APU(SchedulerNew* scheduler, DMA* dma, std::shared_ptr<Config> config)
  : psg1(scheduler)
  , psg2(scheduler)
  , psg3(scheduler)
  , psg4(scheduler, mmio.bias)
  , scheduler(scheduler)
  , dma(dma)
  , config(config)
{ }

void APU::Reset() {
  using namespace common::dsp;

  mmio.fifo[0].Reset();
  mmio.fifo[1].Reset();
  mmio.soundcnt.Reset();
  mmio.bias.Reset();

  resolution_old = 0;
  scheduler->Add(mmio.bias.GetSampleInterval(), event_cb);

  psg1.Reset();
  psg2.Reset();
  psg3.Reset();
  psg4.Reset();

  auto audio_dev = config->audio_dev;

  audio_dev->Close();
  audio_dev->Open(this, (AudioDevice::Callback)AudioCallback);

  buffer = std::make_unique<common::dsp::StereoRingBuffer<float>>(audio_dev->GetBlockSize() * 4, true);

  using Interpolation = Config::Audio::Interpolation;

  switch (config->audio.interpolation) {
    case Interpolation::Cosine:
      resampler.reset(new CosineStereoResampler<float>(buffer));
      break;
    case Interpolation::Cubic:
      resampler.reset(new CubicStereoResampler<float>(buffer));
      break;
    case Interpolation::Sinc_32:
      resampler.reset(new SincStereoResampler<float, 32>(buffer));
      break;
    case Interpolation::Sinc_64:
      resampler.reset(new SincStereoResampler<float, 64>(buffer));
      break;
    case Interpolation::Sinc_128:
      resampler.reset(new SincStereoResampler<float, 128>(buffer));
      break;
    case Interpolation::Sinc_256:
      resampler.reset(new SincStereoResampler<float, 256>(buffer));
      break;
  }

  // TODO: use cubic interpolation or better if M4A samplerate hack is active.
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
        dma->Request(occasion[fifo_id]);
      }
    }
  }
}

void APU::Generate(int cycles_late) {
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

  common::dsp::StereoSample<std::int16_t> sample { 0, 0 };

  constexpr int psg_volume_tab[4] = { 1, 2, 4, 0 };
  constexpr int dma_volume_tab[2] = { 2, 4 };

  auto& psg = mmio.soundcnt.psg;
  auto& dma = mmio.soundcnt.dma;

  auto psg_volume = psg_volume_tab[psg.volume];

  if (config->audio.interpolate_fifo) {
    for (int fifo = 0; fifo < 2; fifo++) {
      latch[fifo] = std::int8_t(fifo_buffer[fifo]->Read() * 127.0);
    }
  }

  for (int channel = 0; channel < 2; channel++) {
    std::int16_t psg_sample = 0;

    if (psg.enable[channel][0]) psg_sample += psg1.sample;
    if (psg.enable[channel][1]) psg_sample += psg2.sample;
    if (psg.enable[channel][2]) psg_sample += psg3.sample;
    if (psg.enable[channel][3]) psg_sample += psg4.sample;

    sample[channel] += psg_sample * psg_volume * psg.master[channel] / 28;

    for (int fifo = 0; fifo < 2; fifo++) {
      if (dma[fifo].enable[channel]) {
        sample[channel] += latch[fifo] * dma_volume_tab[dma[fifo].volume];
      }
    }

    sample[channel] += mmio.bias.level;
    sample[channel]  = std::clamp(sample[channel], (std::int16_t)0, (std::int16_t)0x3FF);
    sample[channel] -= 0x200;
  }

  buffer_mutex.lock();
  resampler->Write({ sample[0] / float(0x200), sample[1] / float(0x200) });
  buffer_mutex.unlock();

  scheduler->Add(mmio.bias.GetSampleInterval() - cycles_late, event_cb);
}

} // namespace nba::core

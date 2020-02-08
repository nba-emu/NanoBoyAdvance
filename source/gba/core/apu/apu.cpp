/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
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

#include <dsp/resampler/cosine.hpp>
#include <dsp/resampler/cubic.hpp>
#include <dsp/resampler/windowed-sinc.hpp>
#include "apu.hpp"
#include "../cpu.hpp"

using namespace GameBoyAdvance;

/* Implemented in callback.cpp */
void AudioCallback(APU* apu, std::int16_t* stream, int byte_len);

APU::APU(CPU* cpu) 
  : psg1(cpu->scheduler)
  , psg2(cpu->scheduler)
  , psg3(cpu->scheduler)
  , psg4(cpu->scheduler, mmio.bias)
  , buffer(new DSP::StereoRingBuffer<float>(4096, true))
  , cpu(cpu)
{ }

void APU::Reset() {
  mmio.fifo[0].Reset();
  mmio.fifo[1].Reset();
  mmio.soundcnt.Reset();
  mmio.bias.Reset();
  
  resolution_old = 0;
  event.countdown = mmio.bias.GetSampleInterval();
  cpu->scheduler.Add(event);
  
  psg1.Reset();
  psg2.Reset();
  psg3.Reset();
  psg4.Reset();
  
  auto audio_dev = cpu->config->audio_dev;
  
  /* TODO: check return value for failure. */
  audio_dev->Close();
  audio_dev->Open(this, (AudioDevice::Callback)AudioCallback);

  using Interpolation = Config::Audio::Interpolation;
    
  switch (cpu->config->audio.interpolation) {
    case Interpolation::Cosine:
      resampler.reset(new DSP::CosineStereoResampler<float>(buffer));
      break;
    case Interpolation::Cubic:
      resampler.reset(new DSP::CubicStereoResampler<float>(buffer));
      break;
    case Interpolation::Sinc_32:
      resampler.reset(new DSP::SincStereoResampler<float, 32>(buffer));
      break;
    case Interpolation::Sinc_64:
      resampler.reset(new DSP::SincStereoResampler<float, 64>(buffer));
      break;
    case Interpolation::Sinc_128:
      resampler.reset(new DSP::SincStereoResampler<float, 128>(buffer));
      break;
    case Interpolation::Sinc_256:
      resampler.reset(new DSP::SincStereoResampler<float, 256>(buffer));
      break;
  }
  
  resampler->SetSampleRates(mmio.bias.GetSampleRate(), audio_dev->GetSampleRate());
}

void APU::OnTimerOverflow(int timer_id, int times) {
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
      latch[fifo_id] = fifo.Read();
      if (fifo.Count() <= 16) {
        cpu->dma.Request(occasion[fifo_id]);
      }
    }
  }
}

void APU::Generate() {
  auto& bias = mmio.bias;
  
  if (bias.resolution != resolution_old) {
    resampler->SetSampleRates(bias.GetSampleRate(), 
      cpu->config->audio_dev->GetSampleRate());
    resolution_old = mmio.bias.resolution;
  }
  
  DSP::StereoSample<std::int16_t> sample { 0, 0 };
  
  /* TODO: what happens if volume=3 (forbidden)? */
  constexpr int psg_volume_tab[4] = { 1, 2, 4, 0 };
  constexpr int dma_volume_tab[2] = { 2, 4 };
  
  auto& psg = mmio.soundcnt.psg;
  auto& dma = mmio.soundcnt.dma;
  
  auto psg_volume = psg_volume_tab[psg.volume];
  
  for (int channel = 0; channel < 2; channel++) {
    std::int16_t psg_sample = 0;
    
    if (psg.enable[channel][0]) psg_sample += psg1.sample;
    if (psg.enable[channel][1]) psg_sample += psg2.sample;
    if (psg.enable[channel][2]) psg_sample += psg3.sample;
    if (psg.enable[channel][3]) psg_sample += psg4.sample;
    
    sample[channel] += psg_sample * psg_volume * psg.master[channel] / 28;

    if (dma[0].enable[channel]) {
      sample[channel] += latch[0] * dma_volume_tab[dma[0].volume];
    }
    
    if (dma[1].enable[channel]) {
      sample[channel] += latch[1] * dma_volume_tab[dma[1].volume];
    }
    
    sample[channel] += mmio.bias.level;
    sample[channel]  = std::clamp(sample[channel], (std::int16_t)0, (std::int16_t)0x3FF);
    sample[channel] -= 0x200;
  }
  
  buffer_mutex.lock();
  resampler->Write({ sample[0] / float(0x200), sample[1] / float(0x200) });
  buffer_mutex.unlock();
  
  event.countdown += bias.GetSampleInterval();
}
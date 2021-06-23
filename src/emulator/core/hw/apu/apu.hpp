/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/dsp/resampler.hpp>
#include <common/dsp/ring_buffer.hpp>
#include <emulator/config/config.hpp>
#include <emulator/core/hw/dma.hpp>
#include <emulator/core/scheduler.hpp>
#include <mutex>

#include "channel/quad_channel.hpp"
#include "channel/wave_channel.hpp"
#include "channel/noise_channel.hpp"
#include "channel/fifo.hpp"
#include "registers.hpp"

namespace nba::core {

class APU {
public:
  APU(Scheduler& scheduler, DMA& dma, std::shared_ptr<Config>);

  void Reset();
  void OnTimerOverflow(int timer_id, int times, int samplerate);

  struct MMIO {
    MMIO(Scheduler& scheduler)
        : psg1(scheduler)
        , psg2(scheduler)
        , psg3(scheduler)
        , psg4(scheduler, bias) {
    }

    FIFO fifo[2];

    QuadChannel psg1;
    QuadChannel psg2;
    WaveChannel psg3;
    NoiseChannel psg4;

    SoundControl soundcnt { fifo, psg1, psg2, psg3, psg4 };
    BIAS bias;
  } mmio;

  std::int8_t latch[2];
  std::shared_ptr<common::dsp::RingBuffer<float>> fifo_buffer[2];
  std::unique_ptr<common::dsp::Resampler<float>> fifo_resampler[2];
  int fifo_samplerate[2];

  std::mutex buffer_mutex;
  std::shared_ptr<common::dsp::StereoRingBuffer<float>> buffer;
  std::unique_ptr<common::dsp::StereoResampler<float>> resampler;

private:
  void StepMixer(int cycles_late);
  void StepSequencer(int cycles_late);

  Scheduler& scheduler;
  DMA& dma;
  std::shared_ptr<Config> config;
  int resolution_old = 0;
};

} // namespace nba::core

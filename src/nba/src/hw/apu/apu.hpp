/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/common/dsp/resampler.hpp>
#include <nba/common/dsp/ring_buffer.hpp>
#include <nba/config.hpp>
#include <mutex>

#include "hw/apu/channel/quad_channel.hpp"
#include "hw/apu/channel/wave_channel.hpp"
#include "hw/apu/channel/noise_channel.hpp"
#include "hw/apu/channel/fifo.hpp"
#include "hw/apu/hle/mp2k.hpp"
#include "hw/apu/registers.hpp"
#include "hw/dma/dma.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct APU {
  APU(
    Scheduler& scheduler,
    DMA& dma,
    Bus& bus,
    std::shared_ptr<Config>
  );

 ~APU();

  void Reset();
  auto GetMP2K() -> MP2K& { return mp2k; }
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

  std::mutex buffer_mutex;
  std::shared_ptr<StereoRingBuffer<float>> buffer;
  std::unique_ptr<StereoResampler<float>> resampler;

private:
  void StepMixer(int cycles_late);
  void StepSequencer(int cycles_late);

  s8 latch[2];
  std::shared_ptr<RingBuffer<float>> fifo_buffer[2];
  std::unique_ptr<Resampler<float>> fifo_resampler[2];
  int fifo_samplerate[2];

  Scheduler& scheduler;
  DMA& dma;
  MP2K mp2k;
  int mp2k_read_index;
  std::shared_ptr<Config> config;
  int resolution_old = 0;
};

} // namespace nba::core

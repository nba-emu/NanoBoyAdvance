/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <mutex>

#include <dsp/resampler.hpp>
#include <dsp/ring_buffer.hpp>

#include "channel/channel_quad.hpp"
#include "channel/channel_wave.hpp"
#include "channel/channel_noise.hpp"
#include "channel/fifo.hpp"
#include "registers.hpp"
#include "../scheduler.hpp"

namespace GameBoyAdvance {

class CPU;

class APU {
public:
  APU(CPU* cpu);
  
  void Reset();
  void OnTimerOverflow(int timer_id, int times);
  void Generate();
  
  Event event { 0, [this]() { this->Generate(); } };
  
  struct MMIO {
    FIFO fifo[2];
    
    SoundControl soundcnt { fifo };
    BIAS bias;
  } mmio;
  
  std::mutex buffer_mutex;
  
  QuadChannel psg1;
  QuadChannel psg2;
  WaveChannel psg3;
  NoiseChannel psg4;
  
  std::int8_t latch[2];
  
  std::shared_ptr<DSP::StereoRingBuffer<float>> buffer;
  std::unique_ptr<DSP::StereoResampler<float>> resampler;
  
private:
  CPU* cpu;
  
  int resolution_old = 0;
};

}
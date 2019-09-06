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

#pragma once

#include "regs.hpp"
#include "sequencer.hpp"
#include "../scheduler.hpp"

#include <dsp/resampler.hpp>
#include <dsp/ring_buffer.hpp>

#include <cstdio>

namespace GameBoyAdvance {

class CPU;

class APU {
public:
  APU(CPU* cpu) 
    : cpu(cpu) 
    , buffer(new DSP::StereoRingBuffer<float>(16384))
    , resampler(new DSP::CosineStereoResampler<float>(buffer))
  { }
  
  void Reset();
  void LatchFIFO(int id, int times);
  void Tick();
  
  Event event { 0, [this]() { this->Tick(); } };
  
  struct MMIO {
    FIFO fifo[2];
    
    SoundControl soundcnt { fifo };
    BIAS bias;
  } mmio;
  
  std::int8_t latch[2];
  
  std::shared_ptr<DSP::StereoRingBuffer<float>> buffer;
  std::unique_ptr<DSP::StereoResampler<float>> resampler;
  
private:
  CPU* cpu;
  
  FILE* dump;
};

}
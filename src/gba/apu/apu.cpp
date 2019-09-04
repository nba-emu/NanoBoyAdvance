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

#include "apu.hpp"
#include "../cpu.hpp"

#ifdef _MSC_VER
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

#include <cstdio>

using namespace GameBoyAdvance;

void AudioCallback(APU* apu, std::int16_t* stream, int byte_len) {
  int samples = byte_len/sizeof(std::int16_t)/2;
  
  for (int x = 0; x < samples; x++) {
    auto sample = apu->buffer->Read() * 32767.0;
    
    stream[x*2+0] = std::int16_t(std::round(sample.left));
    stream[x*2+1] = std::int16_t(std::round(sample.right));
  }
}

void APU::Reset() {
  mmio.fifo[0].Reset();
  mmio.fifo[1].Reset();
  mmio.soundcnt.Reset();
  mmio.bias.Reset();
  
  event.countdown = 512;
  
  dump = fopen("audio.raw", "wb");
  
  // TODO: refactor this out of the core.
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    std::puts("APU: SDL_Init(SDL_INIT_AUDIO) failed.");
    return;
  }
  
  SDL_AudioSpec want;
  
  want.freq = 48000;
  want.samples = 4096;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.callback = (SDL_AudioCallback)AudioCallback;
  want.userdata = this;
  
  SDL_CloseAudio();
  
  if (SDL_OpenAudio(&want, nullptr) < 0) {
    std::puts("SDL_OpenAudio(&want, nullptr) failed.");
    return;
  }
  
  SDL_PauseAudio(0);
}
  
void APU::LatchFIFO(int id, int times) {
  auto& fifo = mmio.fifo[id];
  
  for (int time = 0; time < times; time++) {
    latch[id] = fifo.Read();
    if (fifo.Count() <= 16) {
      cpu->RequestAudioDMA(id);
    }
  }
}

void APU::Tick() {    
  resampler->SetSampleRates(32768.0, 48000.0);
  resampler->Write({ latch[0]/128.0f, latch[1]/128.0f });
  
  event.countdown += 512;
}
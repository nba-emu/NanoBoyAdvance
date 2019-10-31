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

using namespace GameBoyAdvance;

void AudioCallback(APU* apu, std::int16_t* stream, int byte_len) {
  std::lock_guard<std::mutex> guard(apu->buffer_mutex);
  
  int samples = byte_len/sizeof(std::int16_t)/2;
  int available = apu->buffer->Available();
  
  if (available >= samples) {
    for (int x = 0; x < samples; x++) {
      auto sample = apu->buffer->Read() * 32767.0;
      
      stream[x*2+0] = std::int16_t(std::round(sample.left));
      stream[x*2+1] = std::int16_t(std::round(sample.right));
    }
  } else {
    int y = 0;
    
    for (int x = 0; x < samples; x++) {
      auto sample = apu->buffer->Peek(y) * 32767.0;
    
      if (++y == available) y = 0;
      
      stream[x*2+0] = std::int16_t(std::round(sample.left));
      stream[x*2+1] = std::int16_t(std::round(sample.right));
    }
  }
}
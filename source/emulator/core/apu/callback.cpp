/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cmath>

#include "apu.hpp"

namespace nba::core {

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

} // namespace nba::core

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

namespace GameBoyAdvance {

#include <cstdint>
  
/* TODO: support different sample formats? */

class AudioDevice {
public:
  typedef void (*Callback)(void* userdata, std::int16_t* stream, int byte_len);

  virtual auto GetSampleRate() -> int = 0;
  virtual auto GetBlockSize() -> int = 0;
  virtual bool Open(void* userdata, Callback callback) = 0;
  virtual void Close() = 0;
};

class NullAudioDevice : public AudioDevice {
public:
  auto GetSampleRate() -> int final { return 32768; }
  auto GetBlockSize() -> int final { return 4096; }
  bool Open(void* userdata, Callback callback) final { return true; }
  void Close() { }
};

}
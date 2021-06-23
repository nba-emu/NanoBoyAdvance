/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace nba {
  
class AudioDevice {
public:
  virtual ~AudioDevice() {}

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

} // namespace nba

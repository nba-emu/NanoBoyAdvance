// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/integer.hh>

namespace nba {

struct AudioDevice {
  virtual ~AudioDevice() = default;

  typedef void (*Callback)(void* userdata, s16* stream, int byte_len);

  virtual auto GetSampleRate() -> int = 0;
  virtual auto GetBlockSize() -> int = 0;
  virtual bool Open(void* userdata, Callback callback) = 0;
  virtual void SetPause(bool value) = 0;
  virtual void Close() = 0;
};

struct NullAudioDevice : AudioDevice {
  auto GetSampleRate() -> int final { return 32768; }
  auto GetBlockSize() -> int final { return 4096; }
  bool Open(void* userdata, Callback callback) final { return true; }
  void SetPause(bool value) final { }
  void Close() { }
};

} // namespace nba

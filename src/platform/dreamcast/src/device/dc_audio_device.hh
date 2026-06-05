// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/device/audio_device.hh>

#if defined(PLATFORM_DREAMCAST) && __has_include(<kos.h>)
#define NBA_DC_HAS_KOS 1
#include <kos.h>
#include <dc/sound/stream.h>
#else
#define NBA_DC_HAS_KOS 0
#endif

namespace nba {

// Dreamcast audio device using KOS snd_stream for low-latency output.
// Outputs 16-bit stereo PCM at the native Dreamcast rate (44100 Hz).
struct DCAudioDevice : AudioDevice {
  bool Open(void* userdata, Callback callback) override;
  void Close() override;
  void Reset() override;
  auto GetSampleRate() -> int override;
  void SetPause(bool value) override;
  auto IsOpened() const -> bool { return opened_; }

private:
#if NBA_DC_HAS_KOS
  static void* StreamCallback(snd_stream_hnd_t hnd, int req, int* done);

  snd_stream_hnd_t stream_handle_ = SND_STREAM_INVALID;

  // Static instance pointer for the C callback
  static DCAudioDevice* s_instance;
#endif

  Callback callback_ = nullptr;
  void* userdata_ = nullptr;
  bool opened_ = false;
};

} // namespace nba

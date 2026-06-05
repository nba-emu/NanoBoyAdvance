// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "device/dc_audio_device.hh"
#include <algorithm>
#include <cstring>

namespace nba {

#if NBA_DC_HAS_KOS
DCAudioDevice* DCAudioDevice::s_instance = nullptr;
#endif

void DCAudioDevice::SetBufferSize(int bytes) {
  if(bytes == 2048 || bytes == 4096 || bytes == 8192) {
    buffer_size_ = bytes;
  }
}

bool DCAudioDevice::Open(void* userdata, Callback callback) {
  callback_ = callback;
  userdata_ = userdata;

#if NBA_DC_HAS_KOS
  s_instance = this;

  snd_stream_init();
  stream_handle_ = snd_stream_alloc(StreamCallback, buffer_size_);

  if(stream_handle_ == SND_STREAM_INVALID) {
    return false;
  }

  snd_stream_start(stream_handle_, 44100, 1); // 1 = stereo
  opened_ = true;
  return true;
#else
  opened_ = true;
  return true;
#endif
}

void DCAudioDevice::Close() {
#if NBA_DC_HAS_KOS
  if(stream_handle_ != SND_STREAM_INVALID) {
    snd_stream_stop(stream_handle_);
    snd_stream_destroy(stream_handle_);
    stream_handle_ = SND_STREAM_INVALID;
  }
  snd_stream_shutdown();
  s_instance = nullptr;
#endif
  opened_ = false;
}

void DCAudioDevice::Reset() {
#if NBA_DC_HAS_KOS
  if(stream_handle_ != SND_STREAM_INVALID) {
    snd_stream_stop(stream_handle_);
    snd_stream_start(stream_handle_, 44100, 1);
  }
#endif
}

auto DCAudioDevice::GetSampleRate() -> int {
  return 44100;
}

void DCAudioDevice::SetPause(bool value) {
#if NBA_DC_HAS_KOS
  if(stream_handle_ == SND_STREAM_INVALID) return;

  if(value) {
    snd_stream_stop(stream_handle_);
  } else {
    snd_stream_start(stream_handle_, 44100, 1);
  }
#else
  (void)value;
#endif
}

#if NBA_DC_HAS_KOS
void* DCAudioDevice::StreamCallback(snd_stream_hnd_t hnd, int req, int* done) {
  (void)hnd;

  static s16 stream_buffer[8192];

  const int request = std::min(req, static_cast<int>(sizeof(stream_buffer)));

  if(s_instance && s_instance->callback_) {
    s_instance->callback_(s_instance->userdata_, stream_buffer, request);
  } else {
    std::memset(stream_buffer, 0, request);
  }

  *done = request;
  return stream_buffer;
}
#endif

} // namespace nba

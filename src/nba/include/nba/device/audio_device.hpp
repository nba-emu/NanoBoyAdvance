/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>

namespace nba {
  
class AudioDevice {
  public:
    typedef void (*Callback)(void* userdata, s16* stream, int byte_len);

    virtual ~AudioDevice() = default;

    virtual int GetSampleRate() = 0;
    virtual int GetBlockSize() = 0;
    virtual bool Open(void* userdata, Callback callback) = 0;
    virtual void SetPause(bool value) = 0;
    virtual void Close() = 0;
};

class NullAudioDevice : public AudioDevice {
  public:
    int GetSampleRate() final { return 32768; }
    int GetBlockSize() final { return 4096; }
    bool Open(void* userdata, Callback callback) final { return true; }
    void SetPause(bool value) final { }
    void Close() { }
};

} // namespace nba

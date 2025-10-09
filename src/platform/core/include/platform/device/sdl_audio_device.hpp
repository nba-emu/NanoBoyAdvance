/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/device/audio_device.hpp>
#include <SDL3/SDL.h>
#include <vector>

namespace nba {

void SDL3_AudioDeviceCallback(struct SDL3_AudioDevice* audio_device, SDL_AudioStream* stream, int additional_amount, int total_amount);

class SDL3_AudioDevice : public AudioDevice {
  public:
    int GetSampleRate() final;
    int GetBlockSize() final;
    bool Open(void* userdata, Callback callback) final;
    void SetPause(bool value) final;
    void Close() final;

  private:
    friend void SDL3_AudioDeviceCallback(SDL3_AudioDevice* audio_device, SDL_AudioStream* stream, int additional_amount, int total_amount);

    Callback m_callback{};
    void* m_callback_userdata{};
    SDL_AudioStream* m_audio_stream{};
    bool m_opened{};
    bool m_paused{};
    std::vector<u8> m_tmp_buffer{};
};

} // namespace nba

// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <functional>

namespace nba {

class FrameLimiter {
  public:
    explicit FrameLimiter(float fps = 60.f) {
      Reset(fps);
    }

    void Reset();
    void Reset(float fps);
    bool GetFastForward() const;
    void SetFastForward(bool value);
    void SetFastForwardSpeed(int speed);

    void Run(
      std::function<void(void)> frame_advance,
      std::function<void(float)> update_fps
    );

  private:
    static constexpr int k_milliseconds_per_second = 1000;
    static constexpr int k_microseconds_per_second = 1000000;

    int m_frame_count{0};
    int m_frame_duration{};
    float m_frames_per_second{};
    bool m_fast_forward{false};
    int m_fast_forward_speed{0};

    std::chrono::time_point<std::chrono::steady_clock> m_timestamp_target{};
    std::chrono::time_point<std::chrono::steady_clock> m_timestamp_fps_update{};
};

} // namespace nba

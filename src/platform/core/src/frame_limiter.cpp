/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <platform/frame_limiter.hpp>

namespace nba {

void FrameLimiter::Reset() {
  Reset(frames_per_second);
}

void FrameLimiter::Reset(float fps) {
  frame_count = 0;
  frame_duration = int(kMicrosecondsPerSecond / fps);
  frames_per_second = fps;
  fast_forward = false;
  timestamp_target = std::chrono::steady_clock::now();
  timestamp_fps_update = std::chrono::steady_clock::now();
}

auto FrameLimiter::GetFastForward() const -> bool {
  return fast_forward;
}

void FrameLimiter::SetFastForward(bool value) {
  if(fast_forward != value) {
    fast_forward = value;
    if(!fast_forward) {
      timestamp_target = std::chrono::steady_clock::now();
    }
  }
}

void FrameLimiter::SetFastForwardSpeed(int speed) {
  if (speed < 0) {
    speed = 0;
  }
  fast_forward_speed = speed;
}

void FrameLimiter::Run(
  std::function<void(void)> frame_advance,
  std::function<void(float)> update_fps
) {
  if(!fast_forward) {
    timestamp_target += std::chrono::microseconds(frame_duration);
  } else if (fast_forward_speed > 0) {
    timestamp_target += std::chrono::microseconds(frame_duration / fast_forward_speed);
  }

  frame_advance();
  frame_count++;
    
  auto now = std::chrono::steady_clock::now(); 
  auto fps_update_delta = std::chrono::duration_cast<std::chrono::milliseconds>(
    now - timestamp_fps_update).count();

  if(fps_update_delta >= kMillisecondsPerSecond) {
    update_fps(frame_count * float(kMillisecondsPerSecond) / fps_update_delta);
    frame_count = 0;
    timestamp_fps_update = std::chrono::steady_clock::now();
  }

  if(!fast_forward) {
    std::this_thread::sleep_until(timestamp_target);
  } else if (fast_forward_speed > 0) {
    if (now < timestamp_target) {
        std::this_thread::sleep_until(timestamp_target);
    } else {
        timestamp_target = now;
    }
  }
}

} // namespace nba

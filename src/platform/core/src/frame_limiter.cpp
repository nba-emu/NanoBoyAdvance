/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <platform/frame_limiter.hpp>

namespace nba {

void FrameLimiter::Reset() {
  Reset(m_frames_per_second);
}

void FrameLimiter::Reset(float fps) {
  m_frame_count = 0;
  m_frame_duration = int(k_microseconds_per_second / fps);
  m_frames_per_second = fps;
  m_fast_forward = false;
  m_timestamp_target = std::chrono::steady_clock::now();
  m_timestamp_fps_update = std::chrono::steady_clock::now();
}

bool FrameLimiter::GetFastForward() const {
  return m_fast_forward;
}

void FrameLimiter::SetFastForward(bool value) {
  if(m_fast_forward != value) {
    m_fast_forward = value;
    if(!m_fast_forward) {
      m_timestamp_target = std::chrono::steady_clock::now();
    }
  }
}

void FrameLimiter::Run(
  std::function<void(void)> frame_advance,
  std::function<void(float)> update_fps
) {
  if(!m_fast_forward) {
    m_timestamp_target += std::chrono::microseconds(m_frame_duration);
  }

  frame_advance();
  m_frame_count++;
    
  auto now = std::chrono::steady_clock::now(); 
  auto fps_update_delta = std::chrono::duration_cast<std::chrono::milliseconds>(
    now - m_timestamp_fps_update).count();

  if(fps_update_delta >= k_milliseconds_per_second) {
    update_fps(m_frame_count * float(k_milliseconds_per_second) / fps_update_delta);
    m_frame_count = 0;
    m_timestamp_fps_update = std::chrono::steady_clock::now();
  }

  if(!m_fast_forward) {
    std::this_thread::sleep_until(m_timestamp_target);
  }
}

} // namespace nba

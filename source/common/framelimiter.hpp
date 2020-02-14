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

#include <chrono>
#include <functional>
#include <thread>

namespace Common {

class Framelimiter {
public:
  Framelimiter() {
    Reset(60);
  }

  void Reset() {
    Reset(frames_per_second);
  }

  void Reset(float fps) {
    Unbounded(false);
    frames_per_second = fps;
    frame_duration = kMillisecondsPerSecond / fps;
    accumulated_error = 0;
    frame_count = 0;
    timestamp_previous_fps_update = std::chrono::high_resolution_clock::now();
  }

  void Unbounded(bool value) {
    unbounded = value;
  }

  void Run(std::function<void(void)> frame_advance, std::function<void(int)> update_fps) {
    auto timestamp_frame_start = std::chrono::high_resolution_clock::now();

    frame_advance();
    
    auto timestamp_frame_end = std::chrono::high_resolution_clock::now();

    frame_count++;

    auto time_since_last_fps_update = std::chrono::duration_cast<std::chrono::milliseconds>(
      timestamp_frame_end - timestamp_previous_fps_update
    ).count();

    if (time_since_last_fps_update >= kMillisecondsPerSecond) {
      update_fps(frame_count);
      frame_count = 0;
      timestamp_previous_fps_update = timestamp_frame_end;
    }

    if (!unbounded) {
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - timestamp_frame_start
      ).count();

      // NOTE: we need to cast the variables to integers seperately because
      // we don't want the fractional parts to accumulate and overflow into the integer part.
      auto delay = int(frame_duration) + int(accumulated_error) - elapsed;

      std::this_thread::sleep_for(std::chrono::milliseconds(delay));

      accumulated_error -= int(accumulated_error);
      accumulated_error -= std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - timestamp_frame_start
      ).count() - frame_duration;
    }
  }

private:
  static constexpr int kMillisecondsPerSecond = 1000;

  int frame_count = 0;

  float frames_per_second;
  float frame_duration;
  float accumulated_error = 0.0;

  bool unbounded = false;

  std::chrono::time_point<std::chrono::high_resolution_clock> timestamp_previous_fps_update;
};

}
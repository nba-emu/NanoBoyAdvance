/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <functional>
#include <nba/core.hpp>
#include <platform/frame_limiter.hpp>
#include <thread> 

namespace nba {

struct EmulatorThread {
  EmulatorThread(std::unique_ptr<CoreBase>& core);
 ~EmulatorThread();

  bool IsRunning() const;
  bool IsPaused() const;
  void SetPause(bool value);
  bool GetFastForward() const;
  void SetFastForward(bool enabled);
  void SetFrameRateCallback(std::function<void(float)> callback);
  void Start();
  void Stop();

private:
  std::unique_ptr<CoreBase>& core;
  FrameLimiter frame_limiter;
  std::thread thread;
  std::atomic_bool running = false;
  bool paused = false;
  std::function<void(float)> frame_rate_cb;
};

} // namespace nba

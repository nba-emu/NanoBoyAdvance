/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <nba/core.hpp>
#include <platform/frame_limiter.hpp>
#include <thread>

namespace nba {

struct EmulatorThread {
  EmulatorThread(std::unique_ptr<CoreBase>& core);
 ~EmulatorThread();

  bool IsRunning() const;
  bool GetFastForward() const;
  void SetFastForward(bool enabled);
  void Start();
  void Stop();

private:
  std::unique_ptr<CoreBase>& core;
  FrameLimiter frame_limiter;
  std::thread thread;
  std::atomic_bool running = false;
};

} // namespace nba

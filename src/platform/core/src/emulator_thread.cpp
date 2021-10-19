/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <platform/emulator_thread.hpp>

namespace nba {

EmulatorThread::EmulatorThread(
  std::unique_ptr<CoreBase>& core
)   : core(core) {
  frame_limiter.Reset(59.7275);
}

EmulatorThread::~EmulatorThread() {
  if (IsRunning()) Stop();
}

bool EmulatorThread::IsRunning() const {
  return running;
}

bool EmulatorThread::IsPaused() const {
  return paused;
}

void EmulatorThread::SetPause(bool value) {
  paused = value;
}

bool EmulatorThread::GetFastForward() const {
  return frame_limiter.GetFastForward();
}

void EmulatorThread::SetFastForward(bool enabled) {
  frame_limiter.SetFastForward(enabled);
}

void EmulatorThread::SetFrameRateCallback(std::function<void(float)> callback) {
  frame_rate_cb = callback;
}

void EmulatorThread::Start() {
  if (!running) {
    running = true;
    thread = std::thread{[this]() {
      frame_limiter.Reset();

      while (running) {
        frame_limiter.Run([this]() {
          if (!paused) {
            core->RunForOneFrame();
          }
        }, [this](float fps) {
          if (paused) {
            fps = 0;
          }
          frame_rate_cb(fps);
        });
      }
    }};
  }
}

void EmulatorThread::Stop() {
  if (IsRunning()) {
    running = false;
    thread.join();
  }
}

} // namespace nba

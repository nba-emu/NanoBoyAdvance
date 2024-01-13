/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/log.hpp>
#include <platform/emulator_thread.hpp>

namespace nba {

EmulatorThread::EmulatorThread(
  std::unique_ptr<CoreBase>& core
)   : core(core) {
  frame_limiter.Reset(59.7275);
}

EmulatorThread::~EmulatorThread() {
  Stop();
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

void EmulatorThread::SetPerFrameCallback(std::function<void()> callback) {
  per_frame_cb = callback;
}

void EmulatorThread::Start() {
  if(!running) {
    running = true;
    thread = std::thread{[this]() {
      frame_limiter.Reset();

      while(running.load()) {
        ProcessMessages();

        frame_limiter.Run([this]() {
          if(!paused) {
            per_frame_cb();
            core->RunForOneFrame();
          }
        }, [this](float fps) {
          if(paused) {
            fps = 0;
          }
          frame_rate_cb(fps);
        });
      }
    }};
  }
}

void EmulatorThread::Stop() {
  if(IsRunning()) {
    running = false;
    thread.join();
  }
}

void EmulatorThread::Reset() {
  PushMessage({.type = MessageType::Reset});
}

void EmulatorThread::PushMessage(const Message& message) {
  std::lock_guard lock_guard{msg_queue_mutex};
  msg_queue.push(message); // @todo: maybe use emplace.
}

void EmulatorThread::ProcessMessages() {
  // potential optimization: use a separate std::atomic_int to track the number of messages in the queue
  // use atomic_int to do early bail out without acquiring the mutex.
  std::lock_guard lock_guard{msg_queue_mutex};

  while(!msg_queue.empty()) {
    const Message& message = msg_queue.front();

    switch(message.type) {
      case MessageType::Reset: {
        core->Reset();
        break;
      }
      default: Assert(false, "unhandled message type: {}", (int)message.type);
    }

    msg_queue.pop();
  }
}

} // namespace nba

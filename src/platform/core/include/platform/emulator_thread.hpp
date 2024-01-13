/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <functional>
#include <nba/core.hpp>
#include <nba/integer.hpp>
#include <platform/frame_limiter.hpp>
#include <thread>
#include <queue>
#include <mutex>

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
  void SetPerFrameCallback(std::function<void()> callback);
  void Start();
  void Stop();

  void Reset();

private:
  enum class MessageType : u8 {
    Reset
  };

  struct Message {
    MessageType type;
  };

  void PushMessage(const Message& message);
  void ProcessMessages();

  std::queue<Message> msg_queue;
  std::mutex msg_queue_mutex;

  std::unique_ptr<CoreBase>& core;
  FrameLimiter frame_limiter;
  std::thread thread;
  std::atomic_bool running = false;
  bool paused = false;
  std::function<void(float)> frame_rate_cb = [](float) {};
  std::function<void()> per_frame_cb = []() {};
};

} // namespace nba

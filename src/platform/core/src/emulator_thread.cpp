/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/log.hpp>
#include <platform/emulator_thread.hpp>

namespace nba {

EmulatorThread::EmulatorThread() {
  frame_limiter.Reset(k_cycles_per_second / (float)k_cycles_per_subframe);
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

void EmulatorThread::Start(std::unique_ptr<CoreBase> core) {
  Assert(!running, "Started an emulator thread which was already running");

  this->core = std::move(core);
  running = true;

  thread = std::thread{[this]() {
    frame_limiter.Reset();

    while(running.load()) {
      ProcessMessages();

      frame_limiter.Run([this]() {
        if(!paused) {
          // @todo: decide what to do with the per_frame_cb().
          per_frame_cb();
          this->core->Run(k_cycles_per_subframe);
        }
      }, [this](float fps) {
        float real_fps = fps / k_number_of_input_subframes;
        if(paused) {
          real_fps = 0;
        }
        frame_rate_cb(real_fps);
      });
    }

    // Make sure all messages are handled before exiting
    ProcessMessages();
  }};
}

std::unique_ptr<CoreBase> EmulatorThread::Stop() {
  if(IsRunning()) {
    running = false;
    thread.join();
  }

  return std::move(core);
}

void EmulatorThread::Reset() {
  PushMessage({.type = MessageType::Reset});
}

void EmulatorThread::SetKeyStatus(Key key, bool pressed) {
  PushMessage({
    .type = MessageType::SetKeyStatus,
    .set_key_status = {.key = key, .pressed = (u8bool)pressed}
  });
}

void EmulatorThread::PushMessage(const Message& message) {
  // @todo: think of the best way to transparently handle messages
  // sent while the emulator thread isn't running.
  if(!IsRunning()) {
    return;
  }

  if(std::this_thread::get_id() == thread.get_id()) {
    // Messages sent on the emulator thread (i.e. from a callback) do 
    // not need to be pushed to the message queue.
    // Process them right away instead to reduce latency.
    ProcessMessage(message);
  } else {
    std::lock_guard lock_guard{msg_queue_mutex};
    msg_queue.push(message); // @todo: maybe use emplace.
  }
}

void EmulatorThread::ProcessMessages() {
  // potential optimization: use a separate std::atomic_int to track the number of messages in the queue
  // use atomic_int to do early bail out without acquiring the mutex.
  std::lock_guard lock_guard{msg_queue_mutex};

  while(!msg_queue.empty()) {
    ProcessMessage(msg_queue.front());
    msg_queue.pop();
  }
}

void EmulatorThread::ProcessMessage(const Message& message) {
  switch(message.type) {
    case MessageType::Reset: {
      core->Reset();
      break;
    }
    case MessageType::SetKeyStatus: {
      core->SetKeyStatus(message.set_key_status.key, message.set_key_status.pressed);
      break;
    }
    default: Assert(false, "unhandled message type: {}", (int)message.type);
  }
}

} // namespace nba

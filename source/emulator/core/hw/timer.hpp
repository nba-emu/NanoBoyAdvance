/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>
#include <emulator/core/hw/interrupt.hpp>
#include <emulator/core/scheduler.hpp>

#include "apu/apu.hpp"

namespace nba::core {

class Timer {
public:
  Timer(Scheduler* scheduler, InterruptController* irq_controller, APU* apu) : scheduler(scheduler), irq_controller(irq_controller) , apu(apu) { Reset(); }

  void Reset();
  auto Read (int chan_id, int offset) -> std::uint8_t;
  void Write(int chan_id, int offset, std::uint8_t value);

private:
  // TODO: does this really need to return an u32?
  auto GetCounterDeltaSinceLastUpdate(int chan_id) -> std::uint32_t;
  // TODO: get rid of force parameter? It seems fairly useless.
  // Replace Timer with Channel.
  void StartTimer(int chan_id, bool force, int cycles_late);
  void StopTimer(int chan_id);
  void Increment(int chan_id, int increment);

  Scheduler* scheduler;
  InterruptController* irq_controller;
  APU* apu;

  struct Channel {
    int id;
    std::uint16_t reload;
    std::uint32_t counter;

    struct Control {
      int frequency;
      bool cascade;
      bool interrupt;
      bool enable;
    } control;

    bool running;
    int shift;
    int mask;
    int samplerate;
    std::uint64_t timestamp_started;
    Scheduler::Event* event = nullptr;
    std::function<void(int)> event_cb;
  } channels[4];
};

} // namespace nba::core

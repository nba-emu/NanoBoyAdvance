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
  Timer(Scheduler& scheduler, IRQ& irq, APU& apu)
      : scheduler(scheduler)
      , irq(irq)
      , apu(apu) {
    Reset();
  }

  void Reset();
  auto Read (int chan_id, int offset) -> std::uint8_t;
  void Write(int chan_id, int offset, std::uint8_t value);

private:
  enum Registers {
    REG_TMXCNT_L = 0,
    REG_TMXCNT_H = 2
  };

  struct Channel {
    int id;
    std::uint16_t reload = 0;
    std::uint32_t counter = 0;

    struct Control {
      int frequency = 0;
      bool cascade = false;
      bool interrupt = false;
      bool enable = false;
    } control = {};

    bool running = false;
    int shift;
    int mask;
    int samplerate;
    std::uint64_t timestamp_started;
    Scheduler::Event* event = nullptr;
    std::function<void(int)> event_cb;
  } channels[4];

  Scheduler& scheduler;
  IRQ& irq;
  APU& apu;

  auto GetCounterDeltaSinceLastUpdate(Channel const& channel) -> std::uint32_t;
  void StartChannel(Channel& channel, int cycles_late);
  void StopChannel(Channel& channel);
  void OnOverflow(Channel& channel);
};

} // namespace nba::core

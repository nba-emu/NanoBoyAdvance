/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <algorithm>
#include <nba/integer.hpp>
#include <nba/save_state.hpp>

#include "hw/apu/apu.hpp"
#include "hw/irq/irq.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct Timer {
  Timer(Scheduler& scheduler, IRQ& irq, APU& apu);

  void Reset();
  auto ReadByte(int chan_id, int offset) -> u8;
  auto ReadHalf(int chan_id, int offset) -> u16;
  auto ReadWord(int chan_id) -> u32;
  void WriteByte(int chan_id, int offset, u8  value);
  void WriteHalf(int chan_id, int offset, u16 value);
  void WriteWord(int chan_id, u32 value);

  void LoadState(SaveState const& state);
  void CopyState(SaveState& state);

private:
  enum Registers {
    REG_TMXCNT_L = 0,
    REG_TMXCNT_H = 2
  };

  struct Channel {
    int id;
    u16 reload = 0;
    u32 counter = 0;

    struct Pending {
      u16 reload = 0;
      u16 control = 0;
    } pending = {};

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
    u64 timestamp_started;
    Scheduler::Event* event_overflow = nullptr;
  } channels[4];

  Scheduler& scheduler;
  IRQ& irq;
  APU& apu;

  auto ReadCounter(Channel const& channel) -> u16;
  void WriteReload(Channel& channel, u16 value);

  auto ReadControl(Channel const& channel) -> u16;
  void WriteControl(Channel& channel, u16 value);

  void OnReloadWritten(u64 chan_id);
  void OnControlWritten(u64 chan_id);

  auto GetCounterDeltaSinceLastUpdate(Channel const& channel) -> u32;
  void StartChannel(Channel& channel, int cycle_offset);
  void StopChannel(Channel& channel);
  void ReloadCascadeAndRequestIRQ(Channel& channel);
  void OnOverflow(u64 chan_id);
};

} // namespace nba::core

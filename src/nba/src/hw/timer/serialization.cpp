/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "hw/timer/timer.hpp"

namespace nba::core {

// TODO: deduplicate this:
static constexpr int g_ticks_shift[4] = { 0, 6, 8, 10 };
static constexpr int g_ticks_mask[4] = { 0, 0x3F, 0xFF, 0x3FF };

void Timer::LoadState(SaveState const& state) {
  for (int i = 0; i < 4; i++) {
    u16 reload  = state.timer[i].reload;
    u16 control = state.timer[i].control;
    u16 pending_reload  = state.timer[i].pending.reload;
    u16 pending_control = state.timer[i].pending.control;

    channels[i].reload  = reload;
    channels[i].counter = state.timer[i].counter;

    channels[i].control.frequency = control & 3;
    channels[i].control.cascade = control & 4;
    channels[i].control.interrupt = control & 64;
    channels[i].control.enable = control & 128;

    channels[i].shift = g_ticks_shift[channels[i].control.frequency];
    channels[i].mask = g_ticks_mask[channels[i].control.frequency];

    channels[i].running = false;
    channels[i].event_overflow = nullptr;

    if (channels[i].control.enable && !channels[i].control.cascade) {
      // TODO: take care of the one cycle startup-delay:
      StartChannel(channels[i], state.timestamp & channels[i].mask);
    }

    if (pending_reload != reload) {
      WriteReload(channels[i], pending_reload);
    }

    if (pending_control != control) {
      WriteControl(channels[i], pending_control);
    }
  }

  RecalculateSampleRates();
}

void Timer::CopyState(SaveState& state) {
  for (int i = 0; i < 4; i++) {
    state.timer[i].counter = ReadCounter(channels[i]);
    state.timer[i].reload = channels[i].reload;
    state.timer[i].control = ReadControl(channels[i]);
    state.timer[i].pending.reload = channels[i].pending.reload;
    state.timer[i].pending.control = channels[i].pending.control;
  }
}

} // namespace nba::core
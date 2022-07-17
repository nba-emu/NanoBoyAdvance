/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "core.hpp"

namespace nba::core {

void Core::LoadState(SaveState const& state) {
  scheduler.Reset();
  scheduler.SetTimestampNow(state.timestamp);

  cpu.LoadState(state);
  bus.LoadState(state);
  irq.LoadState(state);
  ppu.LoadState(state);
  timer.LoadState(state);
}

void Core::CopyState(SaveState& state) {
  state.magic = SaveState::kMagicNumber;
  state.version = 1;
  state.timestamp = scheduler.GetTimestampNow();

  cpu.CopyState(state);
  bus.CopyState(state);
  irq.CopyState(state);
  ppu.CopyState(state);
  timer.CopyState(state);
}

} // namespace nba::core
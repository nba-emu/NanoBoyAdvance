// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.hpp"

namespace nba::core {

void Core::LoadState(SaveState const& state) {
  scheduler.Reset();
  scheduler.SetTimestampNow(state.timestamp);

  scheduler.LoadState(state);
  cpu.LoadState(state);
  bus.LoadState(state);
  irq.LoadState(state);
  ppu.LoadState(state);
  apu.LoadState(state);
  timer.LoadState(state);
  dma.LoadState(state);
  keypad.LoadState(state);
}

void Core::CopyState(SaveState& state) {
  state.magic = SaveState::kMagicNumber;
  state.version = SaveState::kCurrentVersion;
  state.timestamp = scheduler.GetTimestampNow();

  scheduler.CopyState(state);
  cpu.CopyState(state);
  bus.CopyState(state);
  irq.CopyState(state);
  ppu.CopyState(state);
  apu.CopyState(state);
  timer.CopyState(state);
  dma.CopyState(state);
  keypad.CopyState(state);
}

} // namespace nba::core

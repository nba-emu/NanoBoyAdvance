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
  cpu.LoadState(state);
  bus.LoadState(state);
  irq.LoadState(state);
  ppu.LoadState(state);
}

void Core::CopyState(SaveState& state) {
  cpu.CopyState(state);
  bus.CopyState(state);
  irq.CopyState(state);
  ppu.CopyState(state);
}

} // namespace nba::core
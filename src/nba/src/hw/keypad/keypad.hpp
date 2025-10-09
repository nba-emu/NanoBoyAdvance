/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <nba/save_state.hpp>
#include <nba/scheduler.hpp>
#include <memory>

#include "hw/irq/irq.hpp"

namespace nba::core {

struct KeyPad {
  KeyPad(Scheduler& scheduler, IRQ& irq);

  void Reset();
  void SetKeyStatus(Key key, bool pressed);

  struct KeyInput {
    u16 value = 0x3FF;

    KeyPad* keypad;

    auto ReadByte(uint offset) -> u8;
  } input;

  struct KeyControl {
    u16 mask;
    bool interrupt;

    enum class Mode {
      LogicalOR  = 0,
      LogicalAND = 1
    } mode = Mode::LogicalOR;
  
    KeyPad* keypad;

    auto ReadByte(uint offset) -> u8;
    void WriteByte(uint offset, u8 value);
    void WriteHalf(u16 value);
  } control;

  void LoadState(SaveState const& state);
  void CopyState(SaveState& state);

private:
  void UpdateIRQ();

  Scheduler& scheduler;
  IRQ& irq;
};

} // namespace nba::core

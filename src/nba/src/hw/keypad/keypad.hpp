/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/config.hpp>
#include <memory>

#include "hw/irq/irq.hpp"

namespace nba::core {

struct KeyPad {
  KeyPad(IRQ& irq, std::shared_ptr<Config> config);

  void Reset();

  struct KeyInput {
    u16 value = 0x3FF;

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

private:
  using Key = InputDevice::Key;

  void UpdateInput();
  void UpdateIRQ();

  IRQ& irq;
  std::shared_ptr<Config> config;
};

} // namespace nba::core

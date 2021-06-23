/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

#include "interrupt.hpp"

namespace nba::core {

class SerialBus {
public:
  SerialBus(IRQ& irq) : irq(irq) {}

  void Reset();
  auto Read(std::uint32_t address) -> std::uint8_t;
  void Write(std::uint32_t address, std::uint8_t value);

private:
  std::uint8_t data8;
  std::uint32_t data32;
  std::uint16_t rcnt;

  struct Control {
    enum class ClockSource {
      External = 0,
      Internal = 1
    } clock_source = ClockSource::External;
    // NOTE: this only applies to the internal clock and is ignored when the clock is external.
    enum ClockSpeed {
      _256KHz = 0,
      _2MHz = 1
    } clock_speed = ClockSpeed::_256KHz;
    // TODO: SI/SO state?
    bool busy = false;
    std::uint8_t unused = 0;
    enum Width {
      Byte = 0,
      Word = 1
    } width = Width::Byte;
    bool enable_irq = false;
  } siocnt;

  enum class Mode {
    Normal,
    Multiplay,
    UART,
    GeneralPurpose,
    JOYBUS
  } mode;

  IRQ& irq;
};

} // namespace nba::core

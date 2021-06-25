/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/integer.hpp>

#include "interrupt.hpp"

namespace nba::core {

class SerialBus {
public:
  SerialBus(IRQ& irq) : irq(irq) {}

  void Reset();
  auto Read(u32 address) -> u8;
  void Write(u32 address, u8 value);

private:
  u8 data8;
  u32 data32;
  u16 rcnt;

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
    u8 unused = 0;
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

/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/common/compiler.hpp>

#include "hw/keypad/keypad.hpp"

namespace nba::core {

KeyPad::KeyPad(Scheduler& scheduler, IRQ& irq)
    : scheduler(scheduler)
    , irq(irq) {
  Reset();
}

void KeyPad::Reset() {
  input = {};
  input.keypad = this;
  control = {};
  control.keypad = this;
}

void KeyPad::SetKeyStatus(Key key, bool pressed) {
  const u16 bit = 1 << (int)key;

  if(pressed) {
    input.value &= ~bit;
  } else {
    input.value |=  bit;
  }

  UpdateIRQ();
}

void KeyPad::UpdateIRQ() {
  if(control.interrupt) {
    auto not_input = ~input.value & 0x3FF;
    
    if(control.mode == KeyControl::Mode::LogicalAND) {
      if(control.mask == not_input) {
        irq.Raise(IRQ::Source::Keypad);
      }
    } else if((control.mask & not_input) != 0) {
      irq.Raise(IRQ::Source::Keypad);
    }
  }
}

auto KeyPad::KeyInput::ReadByte(uint offset) -> u8 {
  switch(offset) {
    case 0:
      return u8(value);
    case 1:
      return u8(value >> 8);
  }

  unreachable();
}

auto KeyPad::KeyControl::ReadByte(uint offset) -> u8 {
  switch(offset) {
    case 0: {
      return u8(mask);
    }
    case 1: {
      return ((mask >> 8) & 3) |
              (interrupt ? 64 : 0) |
              (int(mode) << 7);
    }
  }

  unreachable();
}

void KeyPad::KeyControl::WriteByte(uint offset, u8 value) {
  switch(offset) {
    case 0: {
      mask &= 0xFF00;
      mask |= value;
      break;
    }
    case 1: {
      mask &= 0x00FF;
      mask |= (value & 3) << 8;
      interrupt = value & 64;
      mode = Mode(value >> 7);
      break;
    }
    default: {
      unreachable();
    }
  }

  keypad->UpdateIRQ();
}

void KeyPad::KeyControl::WriteHalf(u16 value) {
  mask = value & 0x03FF;
  interrupt = value & 0x4000;
  mode = Mode(value >> 15);
  keypad->UpdateIRQ();
}

} // namespace nba::core

/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/common/compiler.hpp>

#include "hw/keypad.hpp"

namespace nba::core {

KeyPad::KeyPad(IRQ& irq, std::shared_ptr<Config> config)
    : irq(irq)
    , config(config) {
  Reset();
}

void KeyPad::Reset() {
  input = {};
  control = {};
  control.keypad = this;
  config->input_dev->SetOnChangeCallback(std::bind(&KeyPad::UpdateInput, this));
}

void KeyPad::UpdateInput() {
  auto& input_device = config->input_dev;

  input.value = 0;

  if (!input_device->Poll(Key::A)) input.value |= 1;
  if (!input_device->Poll(Key::B)) input.value |= 2;
  if (!input_device->Poll(Key::Select)) input.value |= 4;
  if (!input_device->Poll(Key::Start)) input.value |= 8;
  if (!input_device->Poll(Key::Right)) input.value |= 16;
  if (!input_device->Poll(Key::Left)) input.value |= 32;
  if (!input_device->Poll(Key::Up)) input.value |= 64;
  if (!input_device->Poll(Key::Down)) input.value |= 128;
  if (!input_device->Poll(Key::R)) input.value |= 256;
  if (!input_device->Poll(Key::L)) input.value |= 512;

  UpdateIRQ();
}

void KeyPad::UpdateIRQ() {
  if (control.interrupt) {
    auto not_input = ~input.value & 0x3FF;
    
    if (control.mode == KeyControl::Mode::LogicalAND) {
      if (control.mask == not_input) {
        irq.Raise(IRQ::Source::Keypad);
      }
    } else if ((control.mask & not_input) != 0) {
      irq.Raise(IRQ::Source::Keypad);
    }
  }
}

auto KeyPad::KeyInput::ReadByte(uint offset) -> u8 {
  switch (offset) {
    case 0:
      return u8(value);
    case 1:
      return u8(value >> 8);
  }

  unreachable();
}

auto KeyPad::KeyControl::ReadByte(uint offset) -> u8 {
  switch (offset) {
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
  switch (offset) {
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

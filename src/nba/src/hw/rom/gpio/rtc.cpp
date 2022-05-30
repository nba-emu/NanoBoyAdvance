/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/rom/gpio/rtc.hpp>
#include <nba/log.hpp>
#include <ctime>

#include "hw/irq/irq.hpp"

namespace nba {

constexpr int RTC::s_argument_count[8];

void RTC::Reset() {
  // FIXME: this is a very funky construct.
  // This method should probably be virtual, but it's called
  // from the constructor and calling virtual methods from the constructor
  // is a *very* bad idea.
  GPIO::Reset();

  current_bit = 0;
  current_byte = 0;
  data = 0;
  for (int i = 0; i < 7; i++) {
    buffer[i] = 0;
  }
  port.sck = 0;
  port.sio = 0; 
  port.cs  = 0;
  state = State::Command;

  control.Reset();
}

bool RTC::ReadSIO() {
  data &= ~(1 << current_bit);
  data |= port.sio << current_bit;

  if (++current_bit == 8) {
    current_bit = 0;
    return true;
  }

  return false;
}

auto RTC::ReadPort() -> u8 {
  if (state == State::Sending) {
    return port.sio << static_cast<int>(Port::SIO);
  }

  return 1;
}

void RTC::WritePort(u8 value) {
  int old_sck = port.sck;
  int old_cs  = port.cs;

  if (GetPortDirection(static_cast<int>(Port::CS)) == PortDirection::Out) {
    port.cs = (value >> static_cast<int>(Port::CS)) & 1;
  } else {
    Log<Error>("RTC: CS port should be set to 'output' but configured as 'input'.");;
  }

  if (GetPortDirection(static_cast<int>(Port::SCK)) == PortDirection::Out) {
    port.sck = (value >> static_cast<int>(Port::SCK)) & 1;
  } else {
    Log<Error>("RTC: SCK port should be set to 'output' but configured as 'input'.");
  }

  if (GetPortDirection(static_cast<int>(Port::SIO)) == PortDirection::Out) {
    port.sio = (value >> static_cast<int>(Port::SIO)) & 1;
  }

  // NOTE: this is not tested but probably needed?
  if (!old_cs && port.cs) {
    state = State::Command;
    current_bit  = 0;
    current_byte = 0;
  }

  // TODO: make it clear that SCK is active low!
  if (!port.cs || !(!old_sck && port.sck)) {
    return;
  }

  switch (state) {
    case State::Command: {
      ReceiveCommandSIO();
      break;
    }
    case State::Receiving: {
      ReceiveBufferSIO();
      break;
    }
    case State::Sending: {
      TransmitBufferSIO();
      break;
    }
  }
}

void RTC::ReceiveCommandSIO() {
  bool completed = ReadSIO();

  if (!completed) {
    return;
  }

  // Check whether the command should be interpreted MSB-first or LSB-first.
  if ((data >> 4) == 6) {
    // Fast bit-reversal
    data = (data << 4) | (data >> 4);
    data = ((data & 0x33) << 2) | ((data & 0xCC) >> 2);
    data = ((data & 0x55) << 1) | ((data & 0xAA) >> 1);
    Log<Trace>("RTC: received command in REV format, data=0x{0:X}", data);
  } else if ((data & 15) != 6) {
    Log<Error>("RTC: received command in unknown format, data=0x{0:X}", data);
    return;
  }

  reg = static_cast<Register>((data >> 4) & 7);
  current_bit  = 0;
  current_byte = 0;

  // data[7:] determines whether the RTC register will be read or written.
  if (data & 0x80) {
    ReadRegister();

    if (s_argument_count[(int)reg] > 0) {
      state = State::Sending;
    } else {
      state = State::Command;
    }
  } else {
    if (s_argument_count[(int)reg] > 0) {
      state = State::Receiving;
    } else {
      WriteRegister();
      state = State::Command;
    }
  }
}

void RTC::ReceiveBufferSIO() {
  if (current_byte < s_argument_count[(int)reg] && ReadSIO()) {
    buffer[current_byte] = data;

    if (++current_byte == s_argument_count[(int)reg]) {
      WriteRegister();

      // TODO: does the chip accept more commands or
      // must it be reenabled before sending the next command?
      state = State::Command;
    }
  } 
}

void RTC::TransmitBufferSIO() {
  port.sio = buffer[current_byte] & 1;
  buffer[current_byte] >>= 1;

  if (++current_bit == 8) {
    current_bit = 0;
    if (++current_byte == s_argument_count[(int)reg]) {
      // TODO: does the chip accept more commands or
      // must it be reenabled before sending the next command?
      state = State::Command;
    }
  }
}

void RTC::ReadRegister() {
  switch (reg) {
    case Register::Control: {
      buffer[0] = (control.unknown  ?   2 : 0) |
                  (control.per_minute_irq ? 8 : 0) |
                  (control.mode_24h ?  64 : 0) |
                  (control.poweroff ? 128 : 0);
      break;
    }
    case Register::DateTime: {
      auto timestamp = std::time(nullptr);
      auto time = std::localtime(&timestamp);
      buffer[0] = ConvertDecimalToBCD(time->tm_year - 100);
      buffer[1] = ConvertDecimalToBCD(1 + time->tm_mon);
      buffer[2] = ConvertDecimalToBCD(time->tm_mday);
      buffer[3] = ConvertDecimalToBCD(time->tm_wday);
      buffer[4] = ConvertDecimalToBCD(time->tm_hour);
      buffer[5] = ConvertDecimalToBCD(time->tm_min);
      buffer[6] = ConvertDecimalToBCD(time->tm_sec);
      break;
    }
    case Register::Time: {
      auto timestamp = std::time(nullptr);
      auto time = std::localtime(&timestamp);
      buffer[0] = ConvertDecimalToBCD(time->tm_hour);
      buffer[1] = ConvertDecimalToBCD(time->tm_min);
      buffer[2] = ConvertDecimalToBCD(time->tm_sec);
      break;
    }
  }
}

void RTC::WriteRegister() {
  // TODO: is the datetime register writeable?
  switch (reg) {
    case Register::Control: {
      control.unknown = buffer[0] & 2;
      control.per_minute_irq = buffer[0] & 8;
      control.mode_24h = buffer[0] & 64;
      if (control.per_minute_irq) {
        Log<Error>("RTC: enabled the unimplemented per-minute IRQ.");
      }
      break;
    }
    case Register::ForceReset: {
      // TODO: somehow reset datetime register?
      control.Reset();
      break;
    }
    case Register::ForceIRQ: {
      irq.Raise(core::IRQ::Source::ROM);
      break;
    }
  }
}

} // namespace nba

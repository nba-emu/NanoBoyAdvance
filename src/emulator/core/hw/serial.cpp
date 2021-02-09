/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <common/log.hpp>
#include <emulator/core/cpu-mmio.hpp>

#include "serial.hpp"

namespace nba::core {

void SerialBus::Reset() {
  data8 = 0;
  data32 = 0;
  rcnt = 0;
  siocnt = {};
  mode = Mode::Normal;
}

auto SerialBus::Read(std::uint32_t address) -> std::uint8_t {
  switch (address) {
    // TODO: in multiplayer mode SIODATA32 is aliased to SIOMULTI0-1.
    case SIODATA32_L | 0:
      return data32 & 0xFF;
    case SIODATA32_L | 1:
      return (data32 >> 8) & 0xFF;
    case SIODATA32_H | 0:
      return (data32 >> 16) & 0xFF;
    case SIODATA32_H | 1:
      return (data32 >> 24) & 0xFF;
    case SIODATA8 | 0:
      return data8;
    case RCNT | 0:
      // TODO: bits 0-3 should return current state of SC, SD, SI, SO?
      return rcnt & 0xFF;
    case RCNT | 1:
      return rcnt >> 8;
    default:
      LOG_ERROR("SIO: unhandled read from address 0x{0:08X}", address);
      return 0;
  }
}

void SerialBus::Write(std::uint32_t address, std::uint8_t value) {
  switch (address) {
    // TODO: in multiplayer mode SIODATA32 is aliased to SIOMULTI0-1.
    case SIODATA32_L | 0:
      data32 &= 0xFFFFFF00;
      data32 |= value;
      break;
    case SIODATA32_L | 1:
      data32 &= 0xFFFF00FF;
      data32 |= value << 8;
      break;
    case SIODATA32_H | 0:
      data32 &= 0xFF00FFFF;
      data32 |= value << 16;
      break;
    case SIODATA32_H | 1:
      data32 &= 0x00FFFFFF;
      data32 |= value << 24;
      LOG_TRACE("SIO: SIODATA32 = 0x{0:08X}", data32);
      break;
    case SIOCNT | 0:
      // TODO: update SO during inactivity value.
      siocnt.clock_source = static_cast<Control::ClockSource>(value & 1);
      siocnt.clock_speed = static_cast<Control::ClockSpeed>((value >> 1) & 1);
      if (value & 0x80) {
        LOG_WARN("SIO: triggered data transfer!");
        // NOTE: this is a very rough hack.
        if (siocnt.enable_irq)
          irq.Raise(IRQ::Source::Serial);
      }
      break;
    case SIOCNT | 1:
      siocnt.unused = value & 0xF;
      siocnt.width = static_cast<Control::Width>((value >> 4) & 1);
      siocnt.enable_irq = value & 0x40;
      break;
    case SIODATA8:
      data8 = value;
      LOG_TRACE("SIO: SIODATA8 = 0x{0:02X}", data8);
      break;
    case RCNT | 0:
      rcnt = (rcnt & 0xFF0F) | (value & 0xF0);
      break;
    case RCNT | 1:
      rcnt = (rcnt & 0x3EFF) | ((value << 8) & 0xC100);
      break;
    default:
      LOG_ERROR("SIO: unhandled write to address 0x{0:08X} = 0x{1:02X}", address, value);
  }
}

} // namespace nba::core


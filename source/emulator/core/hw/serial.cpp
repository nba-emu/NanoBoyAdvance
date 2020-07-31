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
      return rcnt & 0xFF;
    case RCNT | 1:
      return rcnt >> 8;
    default:
      LOG_ERROR("Unhandled SIO read from address 0x{0:08X}", address);
      return 0;
  }
}

void SerialBus::Write(std::uint32_t address, std::uint8_t value) {
  switch (address) {
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
      LOG_TRACE("SIODATA32 = 0x{0:08X}", data32);
      break;
    case SIODATA8:
      data8 = value;
      LOG_TRACE("SIODATA8 = 0x{0:02X}", data8);
      break;
    case RCNT | 0:
      break;
    case RCNT | 1:
      break;
    default:
      LOG_ERROR("Unhandled SIO write to address 0x{0:08X} = 0x{1:02X}", address, value);
  }
}

} // namespace nba::core


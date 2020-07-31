/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <common/log.hpp>

#include "serial.hpp"

namespace nba::core {

void SerialBus::Reset() {
}

auto SerialBus::Read(std::uint32_t address) -> std::uint8_t {
  LOG_ERROR("Unhandled SIO read from address 0x{0:08X}", address);
  return 0;
}

void SerialBus::Write(std::uint32_t address, std::uint8_t value) {
  LOG_ERROR("Unhandled SIO write to address 0x{0:08X} = 0x{1:02X}", address, value);
}

} // namespace nba::core


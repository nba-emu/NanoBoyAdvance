/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace nba::core {

class SerialBus {
public:
  void Reset();
  auto Read(std::uint32_t address) -> std::uint8_t;
  void Write(std::uint32_t address, std::uint8_t value);

private:
  std::uint8_t data8;
  std::uint32_t data32;
  std::uint16_t rcnt;
};

} // namespace nba::core

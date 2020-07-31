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
  enum Registers {
    ASDF = 0,
  };
};

} // namespace nba::core

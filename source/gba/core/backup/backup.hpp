/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace GameBoyAdvance { 

class Backup {

public:
  virtual ~Backup() {}

  virtual void Reset() = 0;
  virtual auto Read (std::uint32_t address) -> std::uint8_t = 0;
  virtual void Write(std::uint32_t address, std::uint8_t value) = 0;
};

}
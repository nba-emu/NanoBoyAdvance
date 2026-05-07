/*
 * Copyright (C) 2026 Mireille Meyer
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>

namespace nba {

struct VideoDevice {
  virtual ~VideoDevice() = default;

  virtual void Draw(u32* buffer) = 0;
};

struct NullVideoDevice : VideoDevice {
  void Draw(u32* buffer) final { }
};

} // namespace nba

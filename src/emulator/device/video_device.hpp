/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/integer.hpp>

namespace nba {

class VideoDevice {
public:
  virtual ~VideoDevice() {}

  virtual void Draw(u32* buffer) = 0;
};

class NullVideoDevice : public VideoDevice {
  void Draw(u32* buffer) { }
};

} // namespace nba

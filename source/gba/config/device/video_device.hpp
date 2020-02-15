/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace GameBoyAdvance {

class VideoDevice {
public:
  virtual ~VideoDevice() {}

  virtual void Draw(std::uint32_t* buffer) = 0;
};

class NullVideoDevice : public VideoDevice {
  void Draw(std::uint32_t* buffer) { }
};

}
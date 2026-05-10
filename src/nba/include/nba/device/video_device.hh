// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/integer.hh>

namespace nba {

struct VideoDevice {
  virtual ~VideoDevice() = default;

  virtual void Draw(u32* buffer) = 0;
};

struct NullVideoDevice : VideoDevice {
  void Draw(u32* buffer) final { }
};

} // namespace nba

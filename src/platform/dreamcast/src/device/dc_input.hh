// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/core.hh>

#if defined(PLATFORM_DREAMCAST) && __has_include(<kos.h>)
#define NBA_DC_HAS_KOS 1
#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#else
#define NBA_DC_HAS_KOS 0
#endif

namespace nba {

// Maps Dreamcast controller buttons to GBA Key enums.
// Call PollInput() each frame to update key states on the core.
struct DCInput {
  void PollInput(CoreBase& core);
};

} // namespace nba

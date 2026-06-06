// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dc_config.hh"

#if defined(PLATFORM_DREAMCAST) && __has_include(<arch/arch.h>)
#define NBA_DC_HAS_ARCH 1
#include <arch/arch.h>
#else
#define NBA_DC_HAS_ARCH 0
#endif

namespace nba {

static constexpr size_t kStockDreamcastMaxROMSize = 8 * 1024 * 1024;

inline auto HasExtendedRAM() -> bool {
#if NBA_DC_HAS_ARCH
  return DBL_MEM != 0;
#else
  return false;
#endif
}

inline auto CanLoadLargeROM(DreamcastConfig const& config) -> bool {
  return config.allow_large_roms || HasExtendedRAM();
}

} // namespace nba

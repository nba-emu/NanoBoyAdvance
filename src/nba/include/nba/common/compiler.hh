// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if defined(__clang) || defined(__GNUC__)
  #define ALWAYS_INLINE inline __attribute__((always_inline))
#else
  #define ALWAYS_INLINE inline
#endif

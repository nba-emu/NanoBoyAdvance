/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#if defined(__clang) || defined(__GNUC__)
  #define likely(x)   __builtin_expect((x),1)
  #define unlikely(x) __builtin_expect((x),0)

  #define ALWAYS_INLINE inline __attribute__((always_inline))
#else
  #define likely(x)   (x)
  #define unlikely(x) (x)

  #define ALWAYS_INLINE inline
#endif

#if defined(__clang) || defined(__GNUC__)
  #define unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
  #define unreachable() __assume(0)
#else
  #define unreachable()
#endif
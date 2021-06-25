/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#ifdef __GNUC__
  #define likely(x)   __builtin_expect((x),1)
  #define unlikely(x) __builtin_expect((x),0)
#else
  #define likely(x)   (x)
  #define unlikely(x) (x)
#endif

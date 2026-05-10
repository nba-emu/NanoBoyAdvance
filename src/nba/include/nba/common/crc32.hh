// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <nba/integer.hh>

namespace nba {

inline u32 crc32(u8 const* data, int length) {
  u32 crc32 = 0xFFFFFFFF;

  while(length-- != 0) {
    u8 byte = *data++;

    // TODO: speed this up using a lookup table.
    for(int i = 0; i < 8; i++) {
      if((crc32 ^ byte) & 1) {
        crc32 = (crc32 >> 1) ^ 0xEDB88320;
      } else {
        crc32 >>= 1;
      }
      byte >>= 1;
    }
  }

  return ~crc32;
}

} // namespace nba

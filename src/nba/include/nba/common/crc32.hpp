/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/integer.hpp>

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

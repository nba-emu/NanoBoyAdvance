/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/integer.hpp>
#include <string.h>

namespace common {

template<typename T>
auto read(void* data, uint offset) -> T {
  T value;
  memcpy(&value, (u8*)data + offset, sizeof(T));
  return value;
}

template<typename T>
void write(void* data, uint offset, T value) {
  memcpy((u8*)data + offset, &value, sizeof(T));
}

} // namespace common

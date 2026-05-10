// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/integer.hh>
#include <string.h>

namespace nba {

template<typename T>
auto read(void const* data, uint offset) -> T {
  T value;
  memcpy(&value, (u8*)data + offset, sizeof(T));
  return value;
}

template<typename T>
void write(void* data, uint offset, T value) {
  memcpy((u8*)data + offset, &value, sizeof(T));
}

} // namespace nba

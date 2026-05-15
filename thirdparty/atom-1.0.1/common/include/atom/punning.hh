// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <atom/integer.hh>
#include <string.h>

namespace atom {

  template<typename T>
  auto read(const void* data, uint offset) -> T {
    T value;
    memcpy(&value, (u8*)data + offset, sizeof(T));
    return value;
  }

  template<typename T>
  void write(void* data, uint offset, T value) {
    memcpy((u8*)data + offset, &value, sizeof(T));
  }

} // namespace atom

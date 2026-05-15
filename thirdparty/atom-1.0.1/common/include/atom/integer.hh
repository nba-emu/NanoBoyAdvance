// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <cstdint>
#include <cstddef>

#ifdef ATOM_SCOPE_INTEGER_TYPES
namespace atom {
#endif

  using u8  = std::uint8_t;
  using u16 = std::uint16_t;
  using u32 = std::uint32_t;
  using u64 = std::uint64_t;

  using s8  = std::int8_t;
  using s16 = std::int16_t;
  using s32 = std::int32_t;
  using s64 = std::int64_t;

  using i8  = s8;
  using i16 = s16;
  using i32 = s32;
  using i64 = s64;

  using uint = unsigned int;
  using size_t = std::size_t;

#ifdef ATOM_SCOPE_INTEGER_TYPES
} // namespace atom
#endif

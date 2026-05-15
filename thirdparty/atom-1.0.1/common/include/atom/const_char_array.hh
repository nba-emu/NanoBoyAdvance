// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <atom/integer.hh>
#include <algorithm>

namespace atom {

/**
 * A constant array of char which can be constructed for example from a string literal without decaying to const char*.
 * This is useful for passing string literals as template arguments.
 */
template<size_t N>
struct ConstCharArray {
  static constexpr size_t length = N;

  constexpr ConstCharArray(const char(&src_array)[N]) { // NOLINT(*-explicit-constructor)
    std::copy_n(src_array, N, m_data);
  }

  constexpr char operator[](size_t i) const {
    return m_data[i];
  }

  char m_data[N]{}; // @todo: This should be private, but can't be. Why?
};

} // namespace atom

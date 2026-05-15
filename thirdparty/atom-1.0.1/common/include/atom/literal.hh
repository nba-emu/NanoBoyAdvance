// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

namespace atom::literals {

  constexpr unsigned long long int operator""_KiB(unsigned long long int number) {
    return number * 1024ull;
  }

  constexpr unsigned long long int operator""_MiB(unsigned long long int number) {
    return number * 1024ull * 1024ull;
  }

  constexpr unsigned long long int operator""_GiB(unsigned long long int number) {
    return number * 1024ull * 1024ull * 1024ull;
  }

  constexpr long double operator""_deg(long double angle) {
    return angle * 3.14159265358979323846 / 180.0;
  }

} // namespace atom::literals

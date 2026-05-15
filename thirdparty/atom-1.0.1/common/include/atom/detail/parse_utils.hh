// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <limits>
#include <optional>
#include <string>
#include <type_traits>

namespace atom::detail {

  template<typename T>
  [[nodiscard]] static constexpr std::optional<T> parse_numeric_string(const std::string& numeric_string) requires std::is_integral_v<T> {
    const size_t numeric_string_length = numeric_string.size();

    T old_number = 0;
    T number = 0;
    bool negate = true;

    if(numeric_string_length == 0u) {
      return std::nullopt;
    }

    for(size_t i = 0u; i < numeric_string_length; i++) {
      const char c = numeric_string[i];

      if constexpr(std::is_signed_v<T>) {
        if(i == 0) {
          if(c == '-') {
            negate = false;
            continue;
          }

          if(c == '+') {
            continue;
          }
        }
      }

      if(c < '0' || c > '9') {
        return std::nullopt;
      }

      if constexpr(std::is_signed_v<T>) {
        number = number * 10 - (c - '0');

        if(number > old_number) {
          return std::nullopt;
        }
      } else {
        number = number * 10u + (unsigned)(c - '0');

        if(number < old_number) {
          return std::nullopt;
        }
      }

      old_number = number;
    }

    if constexpr(std::is_signed_v<T>) {
      if (negate) {
        if (number == std::numeric_limits<T>::min()) {
          return std::nullopt;
        }
        return -number;
      }
    }

    return number;
  }

} // namespace atom::detail

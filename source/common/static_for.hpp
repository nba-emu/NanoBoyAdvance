/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <utility>

namespace Common {

namespace Detail {
  template <typename T, T Begin, class Func, T ...Is>
  static constexpr void static_for_impl(Func&& f, std::integer_sequence<T, Is...>) {
    (f(std::integral_constant<T, Begin + Is>{ }), ...);
  }
} // namespace Detail

template <typename T, T Begin, T End, class Func >
static constexpr void static_for(Func&& f) {
  Detail::static_for_impl<T, Begin>(std::forward<Func>(f), std::make_integer_sequence<T, End - Begin>{ });
}

} // namespace Common

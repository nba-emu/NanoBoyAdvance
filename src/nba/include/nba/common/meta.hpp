/*
 * SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <utility>

namespace nba {

namespace detail {

template<typename T, T Begin, class Func, T ...Is>
static constexpr void static_for_impl(Func&& f, std::integer_sequence<T, Is...>) {
  (f(std::integral_constant<T, Begin + Is>{ }), ...);
}

} // namespace nba::detail

template<typename T, T Begin, T End, class Func >
static constexpr void static_for(Func&& f) {
  detail::static_for_impl<T, Begin>(std::forward<Func>(f), std::make_integer_sequence<T, End - Begin>{ });
}

} // namespace nba

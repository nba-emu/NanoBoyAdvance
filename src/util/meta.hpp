/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <utility>

template <typename T, T Begin,  class Func, T ...Is>
constexpr void static_for_impl( Func &&f, std::integer_sequence<T, Is...> ) {
    ( f( std::integral_constant<T, Begin + Is>{ } ),... );
}

template <typename T, T Begin, T End, class Func >
constexpr void static_for( Func &&f ) {
    static_for_impl<T, Begin>( std::forward<Func>(f), std::make_integer_sequence<T, End - Begin>{ } );
}


#pragma once

/* static_for from Stackexchange: 
 * https://codereview.stackexchange.com/questions/173564/implementation-of-static-for-to-iterate-over-elements-of-stdtuple-using-c17
 */
template <typename T, T Begin,  class Func, T ...Is>
constexpr void static_for_impl( Func &&f, std::integer_sequence<T, Is...> ) {
    ( f( std::integral_constant<T, Begin + Is>{ } ),... );
}
template <typename T, T Begin, T End, class Func >
constexpr void static_for( Func &&f ) {
    static_for_impl<T, Begin>( std::forward<Func>(f), std::make_integer_sequence<T, End - Begin>{ } );
}

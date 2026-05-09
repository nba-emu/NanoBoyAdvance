#ifndef TOML11_REFLECT_HPP
#define TOML11_REFLECT_HPP

#include <reflect> // boost-ext/reflect

#include <toml.hpp>

namespace toml
{
namespace refl
{
template<typename T, typename TC>
T from(const basic_value<TC>& v)
{
    T x;
    reflect::for_each([&v, &x](auto I) {
        using member_type = std::remove_cvref_t<decltype(reflect::get<I>(x))>;
        const auto key = std::string(reflect::member_name<I>(x));
        reflect::get<I>(x) = toml::find<member_type>(v, key);
    }, x);

    return x;
}

template<typename TC = toml::type_config, typename T>
basic_value<TC> into(const T& x)
{
    basic_value<TC> v(toml::table{});
    reflect::for_each([&v, &x](auto I) {
        using member_type = std::remove_cvref_t<decltype(reflect::get<I>(x))>;
        const auto key = std::string(reflect::member_name<I>(x));
        v[key] = reflect::get<I>(x);
    }, x);
    return v;
}
} // refl
} // toml

#define TOML11_REFLECT(X) \
    namespace toml {                                       \
    template<>                                             \
    struct into<X>                                         \
    {                                                      \
        template<typename TC = toml::type_config>          \
        static toml::basic_value<TC> into_toml(const X& x) \
        {                                                  \
            return refl::into(x);                          \
        }                                                  \
    };                                                     \
    template<>                                             \
    struct from<X>                                         \
    {                                                      \
        template<typename TC = toml::type_config>          \
        static X from_toml(const toml::basic_value<TC>& v) \
        {                                                  \
            return refl::from<X>(v);                       \
        }                                                  \
    };                                                     \
    } /* toml */

#endif // TOML11_REFLECT_HPP

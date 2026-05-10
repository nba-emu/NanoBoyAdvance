+++
title = "find.hpp"
type  = "docs"
+++

# find.hpp

This function searches for a value in a `toml::value` and performs type conversion if necessary.

{{< hint info >}}

`toml::value` can change the type it stores, and `toml::find` accommodates these types.
Technically, all functions use `toml::basic_value<TC>`.
However, for simplicity, we refer to it as `toml::value` in explanations unless a distinction is necessary.
In the documentation, if the template parameter `TC` changes the type, assume that types like `toml::value::integer_type` will also change accordingly.

{{< /hint >}}

# `toml::find`

## Overview

`toml::find` uses template arguments for the type you want to retrieve and function arguments for the key of the value you want to find.

```cpp
template<typename T, typename TC, typename ... Keys>
T find(const basic_value<TC>& v, Keys ... keys);
```

The supported types for `T` and the behavior of the conversion are the same as for `toml::get`.

If `T` is not specified, a `toml::value` will be returned.

Keys can be of type `toml::value::key_type` or `std::size_t`.
When multiple keys are provided, the function will search recursively through sub-tables or arrays.
If a `toml::value::key_type` is given, `toml::value` is interpreted as a `toml::table`; if a `std::size_t` is given, `toml::value` is interpreted as a `toml::array`.

### Note on Recursive Search

TOML allows for bare keys as well as quoted keys (enclosed in `"` or `'`). Using a quoted key like `"foo.bar" = "baz"` means no sub-table is constructed, and the key is `"foo.bar"`. To handle such patterns, toml11 does not split keys containing `.` and searches using the full string.

Consider the following TOML file:

```toml
[foo]
[foo.bar]
baz = "hoge"

["foo.bar"]
baz = "fuga"
```

The corresponding usage of `toml::find` is shown below:

```cpp
const auto input = toml::parse("input.toml");
const auto baz1 = toml::find<std::string>(input, "foo", "bar", "baz"); // hoge
const auto baz2 = toml::find<std::string>(input, "foo.bar",    "baz"); // fuga
```

cf. [toml.io/en/v1.0.0#keys](https://toml.io/en/v1.0.0#keys)

## `toml::find(value, key)`

Searches for `key` in `value` as if `value` were a `toml::table`, then converts using `toml::get`.

```cpp
template<typename T, typename TC>
/* Equivalent to toml::get<T>(const value&) */ find(
    const basic_value<TC>& v, const typename basic_value<TC>::key_type& ky);

template<typename T, typename TC>
/* Equivalent to toml::get<T>(value&) */ find(
    basic_value<TC>& v, const typename basic_value<TC>::key_type& ky);

template<typename T, typename TC>
/* Equivalent to toml::get<T>(value&&) */ find(
    basic_value<TC>&& v, const typename basic_value<TC>::key_type& ky);
```

If `T` is not specified, the function returns a `toml::value` without conversion.

```cpp
template<typename TC>
basic_value<TC> const& find(
    basic_value<TC> const& v, const typename basic_value<TC>::key_type& ky);

template<typename TC>
basic_value<TC>& find(
    basic_value<TC>& v, const typename basic_value<TC>::key_type& ky);

template<typename TC>
basic_value<TC> find(
    basic_value<TC>&& v, const typename basic_value<TC>::key_type& ky);
```

### Exceptions

If the `toml::value` does not contain a `table`, a `toml::type_error` is thrown.

If the contained `table` does not have the specified element, a `std::out_of_range` is thrown.

If the specified element cannot be converted to `T` (i.e., `toml::get` fails), a `toml::type_error` is thrown.

## `toml::find(value, index)`

Accesses the `index`-th element of `value` as if `value` were a `toml::array`, then converts using `toml::get`.

```cpp
template<typename T, typename TC>
/* Equivalent to toml::get<T>(const value&) */ find(
    const basic_value<TC>& v, const std::size_t index);

template<typename T, typename TC>
/* Equivalent to toml::get<T>(value&) */ find(
    basic_value<TC>& v, const std::size_t index);

template<typename T, typename TC>
/* Equivalent to toml::get<T>(value&&) */ find(
    basic_value<TC>&& v, const std::size_t index);
```

If `T` is not specified, the function returns a `toml::value` without conversion.

```cpp
template<typename TC>
basic_value<TC> const& find(basic_value<TC> const& v, const std::size_t ky);

template<typename TC>
basic_value<TC>& find(basic_value<TC>& v, const std::size_t ky);

template<typename TC>
basic_value<TC> find(basic_value<TC>&& v, const std::size_t ky);
```

### Exceptions

If the `toml::value` does not contain an `array`, a `toml::type_error` is thrown.

If the contained `array` does not have the specified number of elements, a `std::out_of_range` is thrown.

If the specified element cannot be converted to `T` (i.e., `toml::get` fails), a `toml::type_error` is thrown.

## `toml::find(value, keys...)`

```cpp
template<typename T, typename TC, typename K1, typename K2, typename ... Ks>
/* Equivalent to toml::get<T>(const value&) */ find(
    const basic_value<TC>& v, const K1& k1, const K2& k2, const Ks& ... ks);

template<typename T, typename TC, typename K1, typename K2, typename ... Ks>
/* Equivalent to toml::get<T>(value&) */ find(
    basic_value<TC>& v, const K1& k1, const K2& k2, const Ks& ... ks);

template<typename T, typename TC, typename K1, typename K2, typename ... Ks>
/* Equivalent to toml::get<T>(value&&) */ find(
    basic_value<TC>&& v, const K1& k1, const K2& k2, const Ks& ... ks);
```

This function calls `toml::find` recursively.

The failure conditions and the exceptions thrown are the same as those for `toml::find`.

# `toml::find_or(value, key, fallback)`

```cpp
template<typename T, typename TC, typename Key>
T find_or(const basic_value<TC>& v, const Key& key, T&& opt);
```

The `find_or` function takes a default value to avoid throwing an exception when the search fails.

The default value must be of the same type as the return type `T`. Therefore, unlike `toml::find<T>`, `find_or` infers the type `T`.

You can specify `T` explicitly with `find_or<T>`, but this always returns a new value. To obtain a reference, do not specify `T`.

## When `T` is `basic_value`

```cpp
template<typename TC, typename K>
basic_value<TC>& find_or(basic_value<TC>& v, const K& key, basic_value<TC>& opt) noexcept

template<typename TC, typename K>
basic_value<TC> const& find_or(const basic_value<TC>& v, const K& key, const basic_value<TC>& opt) noexcept
```

Searches for the corresponding value and returns it without conversion. Because no conversion is needed, a reference can be returned.

If the value is not found, the default value is returned.

## When `T` is `toml::value::{some_type}`

```cpp
template<typename T, typename TC, typename K>
T& find_or(basic_value<TC>& v, const K& key, T& opt) noexcept

template<typename T, typename TC, typename K>
T const& find_or(const basic_value<TC>& v, const K& key, const T& opt) noexcept
```

Searches for the corresponding value and returns it without conversion. Because no conversion is needed, a reference can be returned.

If the value is not found or if a different type is stored, the default value is returned.

## When `T` is `const char*`

```cpp
template<typename T, typename TC, typename K>
T find_or(const basic_value<TC>& v, const K& key, T opt)
```

Searches for the corresponding value and returns it as a `std::string`.

Since the fallback is constructed from `const char*` to `std::string`, a reference cannot be returned in case of failure.

## When `T` is any other type

```cpp
template<typename T, typename TC, typename K>
T find_or(const basic_value<TC>& v, const K& key, T opt);
```

Searches for the corresponding value and converts it to `T` before returning it.

Because conversion is performed, a reference cannot be returned.

## When multiple keys are provided

```cpp
template<typename Value, typename K1, typename K2, typename K3, typename ... Ks>
auto find_or(Value&& v, const K1& k1, const K2& k2, K3&& k3, Ks&& ... keys) noexcept
 -> decltype(find_or(v, k2, std::forward<K3>(k3), std::forward<Ks>(keys)...))
```

Interprets the last element in the key sequence as the default value and applies `find_or` recursively.

If the inferred type of `T` is `toml::value` or `toml::value::some_type`, a reference can be returned.

If `T` is explicitly specified, conversion is always performed.

# Related

- [get.hpp]({{<ref "get.md">}})
- [value.hpp]({{<ref "value.md">}})

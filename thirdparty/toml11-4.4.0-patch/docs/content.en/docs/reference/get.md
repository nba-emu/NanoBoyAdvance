+++
title = "get.hpp"
type  = "docs"
+++

# get.hpp

These are functions for extracting values from `toml::value` and performing type conversions if necessary.

{{< hint info >}}

`toml::value` can change the type it stores, and `toml::get` accommodates these types.
Technically, all functions use `toml::basic_value<TC>`.
However, for simplicity, we refer to it as `toml::value` in explanations unless a distinction is necessary.
In the documentation, if the template parameter `TC` changes the type, assume that types like `toml::value::integer_type` will also change accordingly.

{{< /hint >}}

# `toml::get<T>`

## Overview

Generally, `toml::get` behaves as follows:
You specify `T` as in `toml::get<int>(v)`.

```cpp
template<typename T, typename TC>
T get(const basic_value<TC>& v);
```

However, depending on the type of `T`, `toml::get` can exhibit different behaviors.

The types of `T` can be categorized into:

1. Types that do not require conversion
2. Types that require conversion

Detailed conditions and the specific types supported are discussed later.

### Types that Do Not Require Conversion

No conversion is needed if the provided `toml::value` is already storing the desired type. For instance, since `toml::value::integer_type` is an alias for `std::int64_t`, `toml::get<std::int64_t>(v)` requires no conversion. In this case, `toml::get` retrieves the `integer` value from `toml::value` and returns a reference to it.

If the provided `toml::value` is a mutable reference (`&`), the returned value is also a mutable reference (`&`). If it is an immutable reference (`const&`), the returned value will also be an immutable reference (`const&`). Returning a mutable reference allows you to overwrite the value stored in `toml::value` through that reference.

### Types that Require Conversion

Types other than the ones mentioned above require conversion. For example, since `toml::value::integer_type` is an alias for `std::int64_t`, `toml::get<std::size_t>(toml::value&)` requires conversion. In this case, `toml::get` retrieves the `integer` value from `toml::value` and casts it to return the appropriate type.

toml11 supports not only simple casts but also complex type conversions like converting from `toml::array` to `std::tuple<int, double, std::string>`, `std::array<double, 4>`, or from `toml::table` to `std::map<std::string, int>`. For specifics, refer to the subsequent sections.

### When Conversion Fails

Sometimes, the expected type conversion cannot be performed. For example, applying `toml::get<int>(v)` to a `toml::value` that holds a `table`.

In such cases, an attempt to convert to the type most similar to the desired type (in this case, `int` using `as_integer`) fails, and a `toml::type_error` is thrown.

When parsing from a file, an error message similar to the following is output:

```
terminate called after throwing an instance of 'toml::type_error'
  what():  toml::value::as_integer(): bad_cast to integer
 --> input.toml
   |
 6 | [fruit]
   | ^^^^^^^-- the actual type is table
```

## When `T` is identical to `toml::value`

```cpp
template<typename T, typename TC>
T& get(basic_value<TC>& v);

template<typename T, typename TC>
T const& get(const basic_value<TC>& v);

template<typename T, typename TC>
T get(basic_value<TC>&& v);
```

Condition:
- `std::is_same<T, basic_value<TC>>` is satisfied.

Since this involves retrieving `toml::value` from `toml::value`, no conversion is performed, and the value is returned as is. This exists solely to generalize the implementation of other functions.

This does not fail.

## When `T` is one of `toml::value::{some_type}`

```cpp
template<typename T, typename TC>
T& get(basic_value<TC>& v);

template<typename T, typename TC>
T const& get(const basic_value<TC>& v);

template<typename T, typename TC>
T get(basic_value<TC>&& v);
```

Condition:
- `T` must be the same as one of the types that `toml::value` can store (e.g., `toml::value::boolean_type`).

If `toml::value` is storing a type that matches the specified type in `toml::get<T>`, such as `toml::value::integer_type`, no type conversion is needed, and a reference can be returned.

If a different type is stored, a `toml::type_error` is thrown.

## When `T` is `basic_value<OtherTC>` with a different `TypeConfig`

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Condition:
- `T` is not `toml::basic_value<TC>`.
- `T` is `toml::basic_value<OtherTC>`.

When a `basic_value` that can store different types is specified, conversion is performed.

Since type conversion occurs, the returned value is a new value and not a reference.

This does not fail (except in cases like memory exhaustion).

## When `T` is an integer type

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Condition:
- `std::is_integral<T>` is satisfied
- `T` is not `bool`
- `T` is not `toml::value::integer_type`

The function assumes that `toml::value` holds an `integer_type`, retrieves its value, converts it to `T`, and returns it.

If a type other than `toml::value::integer_type` is stored, a `toml::type_error` is thrown.

## When `T` is a floating-point type

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Condition:
- `std::is_floating_point<T>` is satisfied
- `T` is not `toml::value::floating_type`

The function assumes that `toml::value` holds a `floating_type`, retrieves its value, converts it to `T`, and returns it.

If a type other than `toml::value::floating_type` is stored, a `toml::type_error` is thrown.

## When `T` is `std::string_view`

This is only available in C++17 and later.

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Condition:
- `std::is_same<std::string_view, T>` is satisfied

The function assumes that `toml::value` holds a `string_type`, retrieves its value, constructs a `std::string_view` from it, and returns it.

If a type other than `toml::value::string_type` is stored, a `toml::type_error` is thrown.

## When `T` is `std::chrono::duration`

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Condition:
- `T` is `std::chrono::duration<Rep, Period>`

The function assumes that `toml::value` holds a `local_time`, retrieves its value, converts it to `std::chrono::duration`, and returns it.

If a type other than `toml::value::local_time` is stored, a `toml::type_error` is thrown.

## When `T` is `std::chrono::system_clock::time_point`

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Condition:
- `std::is_same<T, std::chrono::system_clock::time_point>` is satisfied

If the `toml::value` holds a `local_date`, `local_datetime`, or `offset_datetime`, this function retrieves the value and converts it to `std::chrono::system_clock::time_point`, returning the result.

If the value is of a type other than `local_date`, `local_datetime`, or `offset_datetime`, a `toml::type_error` is thrown.

## When `T` is array-like

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Conditions:
- `T` has an `iterator`
- `T` has a `value_type`
- `T` supports `push_back(x)`
- `T` is not `toml::value::array_type`
- `T` is not `std::string`
- `T` is not `std::string_view`
- `T` is not map-like
- `T` does not have `from_toml()` member function
- `toml::from<T>` is not defined
- A constructor from `toml::basic_value<TC>` is not defined

This includes types like `std::vector<int>` and `std::deque<std::string>`.

If the `toml::value` holds an `array`, this function retrieves the value and converts it to the specified container type, returning the result.

If the value is of a type other than `toml::value::array_type`, a `toml::type_error` is thrown.

## When `T` is `std::array`

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `T` is `std::array<U, N>`

If the `toml::value` holds an `array`, this function retrieves the value and converts it to the specified container type, returning the result.

If the value is of a type other than `toml::value::array_type`, a `toml::type_error` is thrown.

If the `array` held by `toml::value` does not contain enough elements, a `std::out_of_range` is thrown.

## When `T` is `std::forward_list`

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Condition:
- `T` is `std::forward_list<U>`

If the `toml::value` holds an `array`, this function retrieves the value and converts it to a `std::forward_list`, returning the result.

If the value is of a type other than `toml::value::array_type`, a `toml::type_error` is thrown.

## When `T` is `std::pair`

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Condition:
- `T` is `std::pair<T1, T2>`

If the `toml::value` holds an `array`, this function retrieves the value and converts it to `std::pair<T1, T2>`, returning the result.

The `first` and `second` elements are recursively converted.

If the value is of a type other than `basic_value::array_type`, a `toml::type_error` is thrown.

If the `array` held by `toml::value` does not contain exactly 2 elements, a `std::out_of_range` is thrown.

## When `T` is `std::tuple`

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Condition:
- `T` is `std::tuple<T1, T2, ... TN>`

If the `toml::value` holds an `array`, this function retrieves the value and converts it to `std::tuple<T1, T2, ...TN>`, returning the result.

Each element is recursively converted.

If the value is of a type other than `basic_value::array_type`, a `toml::type_error` is thrown.

If the `array` held by `toml::value` does not contain exactly `std::tuple_size<T>::value` elements, a `std::out_of_range` is thrown.

## When `T` is map-like

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Conditions:
- `T` has an `iterator`
- `T` has a `key_type`
- `T` has a `value_type`
- `T` has a `mapped_type`
- `T` is not `toml::value::table_type`
- `T` does not have a `from_toml()` member function
- `toml::from<T>` is not defined
- A constructor from `toml::basic_value<TC>` is not defined

This includes types like `std::map<std::string, int>` and `std::unordered_map<std::string, float>`.

If the `toml::value` holds a `table`, this function retrieves the value and converts it to `T`, returning the result.

Elements are recursively converted.

If the value is of a type other than `basic_value::table_type`, a `toml::type_error` is thrown.

## When `T` is a user-defined type with a specialization of `toml::from<T>`

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Condition:
- A specialization of `toml::from<T>` is defined

If a specialization of `toml::from` for `T` is defined, it is used for type conversion.

Ensure this does not conflict with individually supported types (`std::array`, `std::pair`, `std::tuple` etc).

## When `T` is a user-defined type with a `from_toml` member function

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Conditions:
- `toml::from<T>` is not defined
- `T` has `from_toml()` member function

If `T` has a `from_toml(toml::basic_value<TC>)` member function, it is used for type conversion.

If `toml::from<T>` is defined, it takes precedence.

## When `T` is a user-defined type with a constructor that takes `toml::basic_value<TC>`

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

Conditions:
- `toml::from<T>` is not defined
- `T` does not have `from_toml()` member function
- `T` has a constructor that takes `toml::basic_value<TC>`

If `T` has a constructor that takes `toml::basic_value<TC>`, it is used for type conversion.

If `toml::from<T>` or `T::from_toml` is defined, they take precedence.

# `toml::get_or<T>`

`get_or` takes a default value for use when the conversion fails, avoiding exceptions.

The default value must be of the same type as the target type `T`.
Therefore, unlike `toml::get<T>`, `T` can be inferred in `get_or`.

## When `T` is `basic_value<TC>`

```cpp
template<typename TC>
basic_value<TC> const& get_or(const basic_value<TC>& v, const basic_value<TC>& opt)

template<typename TC>
basic_value<TC> & get_or(basic_value<TC>& v, basic_value<TC>& opt)

template<typename TC>
basic_value<TC> get_or(basic_value<TC>&& v, basic_value<TC>&& opt)
```

Since the conversion target is the same `toml::value`, this never fails.

It exists solely to generalize the implementation of other functions.

## When `T` is `basic_value<TC>::{some_type}`

```cpp
template<typename T, typename TC>
T const& get_or(const basic_value<TC>& v, const T& opt) noexcept

template<typename T, typename TC>
T & get_or(basic_value<TC>& v, T& opt) noexcept

template<typename T, typename TC>     
T get_or(basic_value<TC>&& v, T&& opt) noexcept
```

Performs the same conversion as `toml::get<T>`. If it fails, the second argument is returned.

## When `T` is `const char*`

```cpp
template<typename TC>
typename basic_value<TC>::string_type
get_or(const basic_value<TC>& v,
       const typename basic_value<TC>::string_type::value_type* opt);
```

When `const char*` is passed, the conversion target is interpreted as `std::string`.

## When `T` is something else

```cpp
template<typename TC>
typename std::remove_cv<typename std::remove_reference<T>::type>::type
get_or(const basic_value<TC>& v, T&& opt);
```

Performs the same conversion as `toml::get<T>`. If it fails, the second argument is returned.

# Related

- [find.hpp]({{<ref "find.md">}})
- [from.hpp]({{<ref "from.md">}})
- [value.hpp]({{<ref "value.md">}})


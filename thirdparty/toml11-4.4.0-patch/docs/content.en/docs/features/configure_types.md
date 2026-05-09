+++
title = "configuring types"
type  = "docs"
weight = 50
+++

# Customizing Types

The `toml::value` class uses `std::int64_t` for `integer_type` and `std::unordered_map<key_type, value_type>` for `table_type`.

However, in some cases, you may want to use `boost::multiprecision::int128_t` or `std::map`.

To accommodate this, `toml::value` is implemented with template parameters that allow you to change the stored types.

Just as `std::string` is actually an alias for `std::basic_string<char, std::char_traits<char>, std::allocator<char>>`, `toml::value` is an alias for `toml::basic_value<toml::type_config>`.

Here, we will explain the types contained in `toml::type_config` and how to define a different `config` type.

## `type_config`

The `type_config` class contains the following member types and `static` member functions:

```cpp
namespace toml
{
struct type_config
{
    using comment_type  = preserve_comments;

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    using table_type = std::unordered_map<K, T>;

    static result<integer_type, error_info>
    parse_int(const std::string& str, const source_location src, const std::uint8_t base);

    static result<floating_type, error_info>
    parse_float(const std::string& str, const source_location src, const bool is_hex);
};
}
```

`toml::basic_value<TypeConfig>` defines the stored `boolean_type` as `TypeConfig::boolean_type`, the stored `integer_type` as `TypeConfig::integer_type`, and so on.

Additionally, `array_type` is defined as `TypeConfig::array_type<toml::basic_value<TypeConfig>>` and `table_type` is defined as `TypeConfig::table_type<key_type, toml::basic_value<TypeConfig>>`.

By passing a class that defines these member types and functions to `toml::basic_value`, you can customize the types used by that `toml::basic_value`.

The `parse_int` and `parse_float` functions provide parsing methods when custom numeric types are used. These functions receive strings with prefixes like `0x` and digit separators like `_` removed, such as `123456` or `DEADBEEF`. The `base` parameter will be one of `10`, `16`, `8`, or `2`. Implement these functions to parse your custom `integer_type` and `floating_type`.

As a default implementation, `toml::read_int` and `toml::read_float` are provided.

```cpp
static result<integer_type, error_info>
parse_int(const std::string& str, const source_location src, const std::uint8_t base)
{
    return toml::read_int<integer_type>(str, src, base);
}

static result<floating_type, error_info>
parse_float(const std::string& str, const source_location src, const bool is_hex)
{
    return toml::read_float<floating_type>(str, src, is_hex);
}
```

The `read_int` function uses `istream` and employs `std::hex` and `std::oct` for hexadecimal and octal parsing, respectively. For binary parsing, it is implemented using multiplication and addition. If your type supports these operations, you can use `read_int` as-is.

The `read_float` function also uses `istream`. Hexadecimal floating-point numbers are only supported for `double` and `float` types. If `read_float` is called with any other type and `hexfloat` is used, it will always return a parse error. Therefore, if you need to use a floating-point type other than `double` or `float` with `hexfloat`, you will need to implement support for that. If `hexfloat` is not used, no additional implementation is necessary.

## Preserving Order of Values in Tables

In addition to the default `toml::type_config`, there is also `toml::ordered_type_config`. This changes the `table_type` to an [ordered_map]({{< ref "docs/reference/ordered_map" >}}).

Using this, `toml::ordered_value` is defined, along with aliases for its array and table types as `toml::ordered_array` and `toml::ordered_table`, respectively.

You can use `toml::ordered_value` by calling `toml::parse(...)` as `toml::parse<toml::ordered_type_config>(...)`.

```cpp
#include <toml.hpp>

int main()
{
    toml::ordered_value input = toml::parse<toml::ordered_type_config>("example.toml");
    std::cout << toml::format(input) << std::endl;
    return 0;
}
```

## Not Preserving Comments

The `type_config` defines a container for storing comments via `comment_type`.

If comments do not contain significant information and can be discarded during parsing, specify `toml::discard_comments` for `comment_type`.

```cpp
struct wo_comment_config
{
    using comment_type  = toml::discard_comments; // XXX

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    using table_type = std::unordered_map<K, T>;

    static result<integer_type, error_info>
    parse_int(const std::string& str, const source_location src, const std::uint8_t base)
    {
        return toml::read_int<integer_type>(str, src, base);
    }

    static result<floating_type, error_info>
    parse_float(const std::string& str, const source_location src, const bool is_hex)
    {
        return toml::read_float<floating_type>(str, src, is_hex);
    }
};
```

## Using Containers Other Than `std::vector` for Arrays

To use a container other than `vector` (e.g., `std::deque`) for implementing TOML arrays, modify `array_type` as follows.

Similarly, to use a container other than `unordered_map` (e.g., `std::map`) for table types, modify `table_type` as shown below.

```cpp
struct deque_map_config
{
    using comment_type  = toml::preserve_comments;

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::deque<T>; // XXX
    template<typename K, typename T>
    using table_type = std::map<K, T>; // XXX

    static result<integer_type, error_info>
    parse_int(const std::string& str, const source_location src, const std::uint8_t base)
    {
        return toml::read_int<integer_type>(str, src, base);
    }

    static result<floating_type, error_info>
    parse_float(const std::string& str, const source_location src, const bool is_hex)
    {
        return toml::read_float<floating_type>(str, src, is_hex);
    }
};
```

## Using `boost::multiprecision` for Numeric Types

By using `boost::multiprecision::cpp_int` and `boost::multiprecision::cpp_bin_float_oct`, you can utilize a wider integer type and a more precise floating-point type.

These types implement stream operators, so you can use the default implementations of `read_int` and `read_float` without modification.

```cpp
struct large_num_config
{
    using comment_type  = toml::preserve_comments;

    using boolean_type  = bool;
    using integer_type  = boost::multiprecision::cpp_int;
    using floating_type = boost::multiprecision::cpp_bin_float_oct;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    using table_type = std::unordered_map<K, T>;

    static toml::result<integer_type, toml::error_info>
    parse_int(const std::string& str, const toml::source_location src, const std::uint8_t base)
    {
        return toml::read_int<integer_type>(str, src, base);
    }
    static toml::result<floating_type, toml::error_info>
    parse_float(const std::string& str, const toml::source_location src, const bool is_hex)
    {
        return toml::read_float<floating_type>(str, src, is_hex);
    }
};
```


+++
title = "types.hpp"
type  = "docs"
+++

# types.hpp

This document defines classes that specifies type information.

# `type_config`

`type_config` is a type that encapsulates parameters given to `toml::basic_value`.

When using different types within `toml::basic_value<T>`, you need to define and pass this type separately.
All elements listed are required.

If you use numerical types that cannot use standard stream operators, define and replace the equivalents for `read_int` and `read_float`.

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

using value = basic_value<type_config>;
using table = typename value::table_type;
using array = typename value::array_type;

} // toml
```

## `static` Member Functions

### `parse_int(str, src, base)`

```cpp
static result<integer_type, error_info>
parse_int(const std::string& str, const source_location src, const std::uint8_t base);
```

If you use a type as `integer_type` that cannot utilize standard stream operators, implement this function.
Otherwise, use `read_int` described later.

The `str` parameter receives a string with prefixes, leading zeros, and underscores removed.

The `src` parameter receives a `source_location` pointing to where the string was defined.

The `base` parameter receives one of `10`, `2`, `8`, or `16`.

### `parse_float(str, src, is_hex)`

```cpp
static result<floating_type, error_info>
parse_float(const std::string& str, const source_location src, const bool is_hex);
```

If you use a type as `floating_type` that cannot utilize standard stream operators, implement this function.
Otherwise, use `read_float` described later.

The `str` parameter receives a string with prefixes, leading zeros, and underscores removed.

The `src` parameter receives a `source_location` pointing to where the string was defined.

The `is_hex` parameter indicates whether the format is `hexfloat`. If you don't use the `hexfloat` extension, you don't need to implement this.

For details on the `hexfloat` extension, refer to [spec.hpp]({{<ref "spec.md">}}).

## Non-member Functions

### `read_int`

```cpp
template<typename T>
result<T, error_info>
read_int(const std::string& str, const source_location src, const std::uint8_t base);
```

This is the default function used. It parses using `std::istringstream`.

If `operator>>` and manipulators like `std::hex`, and `std::numeric_limits<T>` are defined (such as for `boost::multiprecision`), you can use this without modifications.

### `read_float`

```cpp
template<typename T>
result<T, error_info>
read_float(const std::string& str, const source_location src, const bool is_hex);
```

This is the default function used. It parses decimals using `std::istringstream` and hexfloats using `sscanf()`.

It supports `double` and `float`.

For other types, if `operator>>` is defined and `hex` is not used, you can use this function.

# `ordered_type_config`

`ordered_type_config` is a variation of `toml::type_config` where the table type is replaced with `toml::ordered_map`.
Additionally, it defines the `toml::ordered_value` alias.

Other than these changes, it is identical to `type_config`.

```cpp
namespace toml
{
struct ordered_type_config
{
    using comment_type  = preserve_comments;

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    using table_type = ordered_map<K, T>;

    static result<integer_type, error_info>
    parse_int(const std::string& str, const source_location src, const std::uint8_t base)
    {
        return read_int<integer_type>(str, src, base);
    }
    static result<floating_type, error_info>
    parse_float(const std::string& str, const source_location src, const bool is_hex)
    {
        return read_float<floating_type>(str, src, is_hex);
    }
};

using ordered_value = basic_value<ordered_type_config>;
using ordered_table = typename ordered_value::table_type;
using ordered_array = typename ordered_value::array_type;

} // toml
```


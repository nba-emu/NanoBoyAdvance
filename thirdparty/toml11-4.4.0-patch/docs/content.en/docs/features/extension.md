+++
title = "extension"
type  = "docs"
weight = 80
+++

# TOML Language Extensions

The TOML language is currently at version v1.0.0, but several new features have been discussed and merged, with ongoing discussions for v1.1.0.

Among the proposed features, some were deemed to have limited use cases, some faced implementation challenges in their proposed form, and others were not adopted at all.

In toml11, we have experimentally implemented a selection of these features. Please note that these features are supported in toml11 but are not supported by other parsers and are unlikely to be supported in the future.

Additionally, these features are disabled by default. To use them, you must explicitly set the corresponding feature flags to `true`. This design choice ensures that non-standard features are only used intentionally.

Some of these features may eventually be merged into the TOML language itself. If a feature is officially adopted, the corresponding experimental implementation in toml11 may be removed in a minor version update after the official feature is implemented.

## `null`

This feature allows the use of `null` as a value in TOML files.

```
a = null
b = [ 1, 2, 3, null, 5]
```

To enable this, set the `ext_null_value` flag in `toml::spec` to `true`.

When parsed, it will be treated as `toml::value_t::empty`, similar to a default-constructed value. However, the location information within the file will be set.

`null` is parsed only in the context of values. Therefore, if `null` is used as a key, it will be interpreted as the string `"null"`, as it has been in the standard TOML.

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec;
    spec.ext_null_value = true;

    const auto v = toml::parse_str("a = null", spec);

    assert(v.at("a").is_empty());
    assert(v.at("a").is(toml::value_t::empty));

    return 0;
}
```

## Hexadecimal Format for Floating-Point Numbers

This feature allows the use of hexadecimal format for floating-point numbers in TOML files.

```
a = 0x1.91eb851eb851fp+1 # 3.14
```

To enable this, set the `ext_hex_float` flag in `toml::spec` to `true`.

The format follows the `printf` specification for `%a/%A`.

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec;
    spec.ext_hex_float = true;

    const auto v = toml::parse_str("a = 0x1.91eb851eb851fp+1", spec);

    assert(v.at("a").is_floating());
    assert(v.at("a").as_floating() == 3.14);

    return 0;
}
```

## Suffixes for Integers and Floating-Point Numbers

This feature allows the use of suffixes for numbers in TOML files. It can be applied to integers and floating-point numbers in decimal notation.

This is particularly useful for displaying units.

```
a = 86_400_sec
b = 3.1416_rad
c = 10_μm
```

However, these are purely suffixes and do not perform any unit conversion. If unit conversion is needed, users should implement it by referencing the suffix.

To enable this, set the `ext_num_suffix` flag in `toml::spec` to `true`.

The suffix must be separated from the number by an underscore (`_`).

For clarity, the suffix cannot start with a digit.

```
distance = 100_m  # valid
distance = 10_0m  # invalid
distance = 10_0_m # valid
```

The suffix is stored as `std::string suffix` in the format information.

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec;
    spec.ext_hex_float = true;

    const auto v = toml::parse_str("a = 86_400_sec", spec);

    assert(v.at("a").is_integer());
    assert(v.at("a").as_integer() == 86400);
    assert(v.at("a").as_integer_fmt().suffix == "sec");

    return 0;
}
```

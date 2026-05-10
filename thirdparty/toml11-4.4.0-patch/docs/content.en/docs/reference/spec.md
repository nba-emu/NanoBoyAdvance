+++
title = "spec.hpp"
type  = "docs"
+++

# spec.hpp

`spec.hpp` defines classes for specifying the version of TOML.

# `toml::semantic_version`

`semantic_version` is a class that stores version information.

```cpp
namespace toml
{
struct semantic_version
{
    constexpr semantic_version(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept;

    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
};

constexpr semantic_version
make_semver(std::uint32_t major, std::uint32_t minor, std::uint32_t patch) noexcept;

constexpr bool operator==(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator!=(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator< (const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator<=(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator> (const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator>=(const semantic_version&, const semantic_version&) noexcept;

std::ostream& operator<<(std::ostream& os, const semantic_version& ver);
} //toml
```

## Member Functions

### Constructor

```cpp
constexpr semantic_version(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept;
```

Constructs a `semantic_version` instance with the specified `major`, `minor`, and `patch` version numbers.

## Non-Member Functions

### Comparison Operators

```cpp
constexpr bool operator==(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator!=(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator< (const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator<=(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator> (const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator>=(const semantic_version&, const semantic_version&) noexcept;
```

Compares two `semantic_version` instances according to semantic versioning rules.

### Stream Operator

```cpp
std::ostream& operator<<(std::ostream& os, const semantic_version& ver);
```

Outputs the version in the format `{major}.{minor}.{patch}`.

### `to_string`

```cpp
std::string to_string(const semantic_version& ver);
```

Converts the version to a string in the format `{major}.{minor}.{patch}`.

# `toml::spec`

`spec` is a class that stores TOML version information.

```cpp
struct spec
{
    constexpr static spec default_version() noexcept;

    constexpr static spec v(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept;

    constexpr explicit spec(const semantic_version& semver) noexcept;

    semantic_version version; // toml version

    // diff from v1.0.0 -> v1.1.0
    bool v1_1_0_allow_control_characters_in_comments;
    bool v1_1_0_allow_newlines_in_inline_tables;
    bool v1_1_0_allow_trailing_comma_in_inline_tables;
    bool v1_1_0_allow_non_english_in_bare_keys;
    bool v1_1_0_add_escape_sequence_e;
    bool v1_1_0_add_escape_sequence_x;
    bool v1_1_0_make_seconds_optional;

    // library extensions
    bool ext_hex_float;  // allow hex float
    bool ext_num_suffix; // allow number suffix
    bool ext_null_value; // allow null value
};
```

## Member Functions

### Constructor

```cpp
constexpr explicit spec(const semantic_version& semver) noexcept;
```

Constructs a `spec` with the specified TOML version.

Supports TOML v1.0.0 and TOML v1.1.0.

### `default_version()`

```cpp
constexpr static spec default_version() noexcept;
```

Constructs a `spec` with the default version.

Used as the default value for `toml::parse` and `toml::format`.

In toml11 v4.4.0, the value is v1.0.0.

### `v(major, minor, patch)`

```cpp
constexpr static spec v(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept;
```

Constructs a `spec` with the specified version.

## Member Variables

Each flag is automatically set to `true` when the specified version includes the corresponding feature.

You can modify these flags to change the behavior of `toml::parse` and `toml::format`.

{{<hint warning>}}

Some features of TOML v1.1.0 are still under fairly lengthy discussion and may still be reverted.

If they are indeed reverted, toml11 will remove those features in a minor version upgrade or move them to a corresponding later version.

As such, any features related to future versions should be considered unstable.

{{</hint>}}

### Example

```cpp
auto spec = toml::spec::v(1, 0, 0);
// Allow newlines in inline tables in addition to v1.0.0 features.
// Other v1.1.0 features are not enabled.
spec.v1_1_0_allow_newlines_in_inline_tables = true;

auto input = toml::parse("input_file.toml", spec);
```

### `v1_1_0_allow_control_characters_in_comments`

```cpp
bool v1_1_0_allow_control_characters_in_comments;
```

Allows most control characters in comments.

Added in TOML v1.1.0.

### `v1_1_0_allow_newlines_in_inline_tables`

```cpp
bool v1_1_0_allow_newlines_in_inline_tables;
```

Allows newlines in inline tables.

Added in TOML v1.1.0.

### `v1_1_0_allow_trailing_comma_in_inline_tables`

```cpp
bool v1_1_0_allow_trailing_comma_in_inline_tables;
```

Allows trailing commas in inline tables.

Added in TOML v1.1.0.

### `v1_1_0_add_escape_sequence_e`

```cpp
bool v1_1_0_add_escape_sequence_e;
```

Allows `\e` to represent the ESC character.

Added in TOML v1.1.0.

### `v1_1_0_add_escape_sequence_x`

```cpp
bool v1_1_0_add_escape_sequence_x;
```

Allows `\xHH` to represent a single byte character.

Added in TOML v1.1.0.

### `v1_1_0_make_seconds_optional`

```cpp
bool v1_1_0_make_seconds_optional;
```

Makes the seconds component in time optional.

Unspecified seconds default to `0`.

Added in TOML v1.1.0.

### `ext_hex_float`

```cpp
bool ext_hex_float;
```

This is a language extension specific to toml11.

It is initialized to `false` regardless of the specified version.
You must explicitly set it to `true` if you want to use it.

Allows hexadecimal representation of floating-point numbers.

The hexadecimal representation conforms to the `printf` format specifier `%a/%A`.

```
hexf = 0xC0FFEEp-10
```

`toml::format` will format using hexadecimal notation only if the passed `toml::spec` has `ext_hex_float` set to `true`.
If the format specifier indicates `hex` but the `toml::spec` passed to `toml::format` has `ext_hex_float` set to `false`, the hexadecimal specification is ignored, and the number is output in decimal notation with maximum precision.

### `ext_num_suffix`

```cpp
bool ext_num_suffix;
```

This is a language extension specific to toml11.

It is initialized to `false` regardless of the specified version.
You must explicitly set it to `true` if you want to use it.

Allows the addition of suffixes to decimal integers and floating-point numbers. This does not apply to hexadecimal, octal, or binary notations.

There must be an `_` separator between the number and the suffix.

The suffix cannot start with a digit to avoid confusion with the numeric part.

```
distance = 10_m   # valid
distance = 10_2m  # invalid
distance = 10_2_m # valid
```

The suffix is stored in the format information as `std::string suffix`.
The `_` separating the number from the suffix is not included in the `suffix`.

```cpp
toml::value distance = toml::find(input, "distance");
assert(distance.as_integer_fmt().suffix == std::string("m"));
```

`toml::format` will format the value with the suffix only if the passed `toml::spec` has `ext_num_suffix` set to `true`.

The `suffix` follows the grammar defined as:

```abnf
non-digit-graph = ALPHA / non-ascii
graph           = ALPHA / DIGIT / non-ascii
suffix          = _ non-digit-graph *( graph / ( _ graph ) )
```

### `ext_null_value`

```cpp
bool ext_null_value;
```

This is a language extension specific to toml11.

Allows the use of `null` as a value.

A `toml::value` specified as `null` will have no value, and `is_empty()` will return `true`.

`toml::format` will format it as `null` only if the passed `toml::spec` has `ext_null_value` set to `true`.
Otherwise, `toml::format` will terminate with an error.

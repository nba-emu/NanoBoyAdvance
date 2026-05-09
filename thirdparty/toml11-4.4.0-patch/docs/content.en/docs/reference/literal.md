+++
title = "literal.hpp"
type  = "docs"
+++

# literal.hpp

In `literal.hpp`, the `_toml` literal is defined.

The `_toml` literal parses string literals into `toml::value`.

```cpp
namespace toml
{
inline namespace literals
{
inline namespace toml_literals
{
toml::value operator"" _toml(const char* str, std::size_t len);
toml::value operator"" _toml(const char8_t* str, std::size_t len); // Available in C++20 and later
} // toml_literals
} // literals
} // toml
```

## Free Functions

### `operator"" _toml(const char*)`

```cpp
toml::value operator"" _toml(const char* str, std::size_t len);
```

Converts a string literal into a `toml::value` by parsing it.

For typical TOML files, this performs equivalent parsing to `toml::parse`.

```cpp
const auto v1 = "a = 'foo'"_toml; // v1: {a = 'foo'}
```

When dealing with multiline content, raw string literal is convenient.

```cpp
const auto v1 = R"(
    a = 42
    b = "foo"
)"_toml;
```

If the value is a standalone entity, it represents that value.

```cpp
const auto v2 = "'foo'"_toml;     // v2: 'foo'
```

TOML allows keys consisting solely of numbers. When distinguishing between table definitions and arrays is ambiguous (e.g., `[1]`), table definitions take precedence.

To interpret as an array, use a trailing comma.

```cpp
const auto v3 = "[1]"_toml;  // v3: {1 = {}}
const auto v4 = "[1,]"_toml; // v4: [1,]
```

### `operator"" _toml(const char8_t*)`

Defined when `char8_t` is available. Otherwise identical in functionality, differing only in argument type.

# Example

```cpp
#include <toml.hpp>
int main()
{
    using namespace toml::literals::toml_literals;
    const auto v = "a = \"foo\""_toml;
    assert(v.at("a").as_string() == "foo");
    return 0;
}
```


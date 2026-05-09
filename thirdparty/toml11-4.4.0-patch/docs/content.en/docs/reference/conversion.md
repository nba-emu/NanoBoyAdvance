+++
title = "conversion.hpp"
type  = "docs"
+++

# conversion.hpp

Provides macros to automatically define conversion functions for supporting user-defined types with `toml::get` and `toml::find`.

```cpp
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(NAME, ...)
```

## Example

```cpp
namespace foo
{
struct Foo
{
    std::string s;
    double      d;
    int         i;
};
} // foo

TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(foo::Foo, s, d, i)
```

# Related

- [from.hpp]({{<ref "from.md">}})
- [into.hpp]({{<ref "into.md">}})

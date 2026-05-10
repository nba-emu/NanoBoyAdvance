+++
title = "from.hpp"
type  = "docs"
+++

# from.hpp

Defines a `struct` used for conversion from `toml::value` in `toml::get` and `toml::find`.

You can achieve the same functionality by adding a `from_toml` member function, but for classes where you cannot add member functions, use `from<T>`.

This file does not provide specific implementations. Please specialize this `struct` when using.

```cpp
namespace toml
{

template<typename T>
struct from;

} // toml
```

## Example

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib

#include <toml11/from.hpp>

namespace toml
{
template<>
struct from<extlib::foo>
{
    template<typename TC>
    static extlib::foo from_toml(const toml::basic_value<TC>& v)
    {
        return extlib::foo{toml::find<int>(v, "a"), toml::find<std::string>(v, "b")};
    }
};
} // toml
```

# Related

- [conversion.hpp]({{<ref "conversion.md">}})
- [into.hpp]({{<ref "into.md">}})


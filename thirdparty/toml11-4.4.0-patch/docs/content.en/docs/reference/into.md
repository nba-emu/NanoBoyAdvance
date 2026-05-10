+++
title = "into.hpp"
type  = "docs"
+++

# into.hpp

Defines a `struct` used for conversion from user-defined types into `toml::value` constructors.

You can achieve the same functionality by adding an `into_toml` member function, but for classes where you cannot add member functions, use `into<T>`.

This file does not provide specific implementations. Please specialize this `struct` when using.

```cpp
namespace toml
{

template<typename T>
struct into;

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

#include <toml11/into.hpp>

namespace toml
{
template<>
struct into<extlib::foo>
{
    template<typename TC>
    static toml::basic_value<TC> into_toml(const extlib::foo& f)
    {
        using value_type = toml::basic_value<TC>;
        using table_type = typename value_type::table_type;
        return value_type(table_type{{"a", f.a}, {"b", f.b}});
    }
};
} // toml
```

# Related

- [conversion.hpp]({{<ref "conversion.md">}})
- [from.hpp]({{<ref "from.md">}})


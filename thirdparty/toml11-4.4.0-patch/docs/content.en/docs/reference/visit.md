+++
title = "visit.hpp"
type  = "docs"
+++

# visit.hpp

In `visit.hpp`, `toml::visit` is defined.

# `toml::visit`

## Functions

```cpp
namespace toml
{
template<typename Visitor, typename ... Args>
/* Return value when Visitor is called with a value of basic_value<TC> */
visit(Visitor&& visitor, Args&& ... args);
}
```

`toml::visit` calls the overload of `Visitor` corresponding to the type held by `basic_value<TC>`, and returns the result.

#### Requirements

`Visitor` must be a function or function object callable with any type held by `basic_value<TC>`.

Additionally, the return value must be consistent across all overloads.

#### Example

```cpp
#include <toml.hpp>
#include <iostream>

struct type_name_of
{
    std::string operator()(const toml::value::boolean_type        &) const {return "boolean";}
    std::string operator()(const toml::value::integer_type        &) const {return "integer";}
    std::string operator()(const toml::value::floating_type       &) const {return "floating";}
    std::string operator()(const toml::value::string_type         &) const {return "string";}
    std::string operator()(const toml::value::local_time_type     &) const {return "local_time";}
    std::string operator()(const toml::value::local_date_type     &) const {return "local_date";}
    std::string operator()(const toml::value::local_datetime_type &) const {return "local_datetime";}
    std::string operator()(const toml::value::offset_datetime_type&) const {return "offset_datetime";}
    std::string operator()(const toml::value::array_type          &) const {return "array";}
    std::string operator()(const toml::value::table_type          &) const {return "table";}
};

int main()
{
    toml::value v(3.14);
    std::cout << toml::visit(type_name_of{}, v) << std::endl; // floating
    return 0;
}
```

# Related

- [value.hpp]({{<ref "value.md">}})

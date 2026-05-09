+++
title = "value_t.hpp"
type  = "docs"
+++

# value_t.hpp

# `value_t`

`value_t` is used to handle the type information of `toml::value`.

```cpp
namespace toml
{
enum class value_t : std::uint8_t
{
    empty           =  0,
    boolean         =  1,
    integer         =  2,
    floating        =  3,
    string          =  4,
    offset_datetime =  5,
    local_datetime  =  6,
    local_date      =  7,
    local_time      =  8,
    array           =  9,
    table           = 10
};

std::ostream& operator<<(std::ostream& os, value_t t);
std::string to_string(value_t t);
} // toml
```

## Non-member Functions

### Stream Operator

```cpp
std::ostream& operator<<(std::ostream& os, value_t t);
```

Outputs the string representation of the `value_t` to the stream.

### `to_string`

```cpp
std::string to_string(value_t t);
```

Returns the string representation of the `value_t`.

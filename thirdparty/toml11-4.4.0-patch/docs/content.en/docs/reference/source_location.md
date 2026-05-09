+++
title = "source_location.hpp"
type  = "docs"
+++

# source_location.hpp

`source_location.hpp` defines a class representing a specific area within a TOML file.

This class is used to represent problematic areas in error messages.

# `toml::source_location`

`source_location` is a class representing a specific area within a TOML file.

```cpp
namespace toml
{
struct source_location
{
  public:

    explicit source_location(/* implementation-defined */);
    ~source_location() = default;
    source_location(source_location const&) = default;
    source_location(source_location &&)     = default;
    source_location& operator=(source_location const&) = default;
    source_location& operator=(source_location &&)     = default;

    bool        is_ok()  const noexcept;
    std::size_t length() const noexcept;

    std::size_t first_line_number()   const noexcept;
    std::size_t first_column_number() const noexcept;
    std::size_t last_line_number()    const noexcept;
    std::size_t last_column_number()  const noexcept;

    std::string const& file_name()    const noexcept;

    std::size_t num_lines()           const noexcept;

    std::string const& first_line() const;
    std::string const& last_line()  const;

    std::vector<std::string> const& lines() const noexcept;
};

template<typename ... Ts>
std::string format_location(const source_location& loc, const std::string& msg, const Ts& ... locs_and_msgs);
} //toml
```

## Member Functions

### Constructor

```cpp
explicit source_location(/* implementation-defined */);
```

`toml::source_location` can only be constructed via `toml::parse` or the `_toml` literal.

### `is_ok()`

```cpp
bool is_ok() const noexcept;
```

Returns `true` if the `source_location` holds a valid value, `false` otherwise.

The result of `location()` from `toml::value` constructed outside of `toml::parse` or `_toml` literals returns `false` for `is_ok` as it points to nothing.

### `length()`

```cpp
std::size_t length() const noexcept;
```

Returns the length of the area pointed to by the `source_location`.

Returns `0` if it does not hold a valid value.

### `first_line_number()`

```cpp
std::size_t first_line_number() const noexcept;
```

Returns the line number of the first line of the area pointed to by the `source_location`.

Returns `1` if it does not hold a valid value.

### `first_column_number()`

```cpp
std::size_t first_column_number() const noexcept;
```

Returns the column number of the first column of the area pointed to by the `source_location`.

Returns `1` if it does not hold a valid value.

### `last_line_number()`

```cpp
std::size_t last_line_number() const noexcept;
```

Returns the line number of the last line of the area pointed to by the `source_location`.

Returns `1` if it does not hold a valid value.

### `last_column_number()`

```cpp
std::size_t last_column_number() const noexcept;
```

Returns the column number of the last column of the area pointed to by the `source_location`.

Returns `1` if it does not hold a valid value.

### `file_name()`

```cpp
std::string const& file_name() const noexcept;
```

Returns the file name containing the area pointed to by the `source_location`.

Returns `"unknown file"` if it does not hold a valid value.

### `num_lines()`

```cpp
std::size_t num_lines() const noexcept;
```

Returns the number of lines in the area pointed to by the `source_location`.

Returns `0` if it does not hold a valid value.

### `first_line()`

```cpp
std::string const& first_line() const;
```

Returns the first line of the area pointed to by the `source_location`.

Throws `std::out_of_range` if it does not hold a valid value.

### `last_line()`

```cpp
std::string const& last_line() const;
```

Returns the last line of the area pointed to by the `source_location`.

Throws `std::out_of_range` if it does not hold a valid value.

### `lines()`

```cpp
std::vector<std::string> const& lines() const noexcept;
```

Returns all lines in the area pointed to by the `source_location`.

Returns a reference to an empty `std::vector` if it does not hold a valid value.

## Non-Member Functions
 
### `format_location`

```cpp
template<typename ... Ts>
std::string format_location(const source_location& loc, const std::string& msg,
                            const Ts& ... locs_and_msgs);
```

Formats the specified `source_location` and its associated message as follows:

```
 -> {filename.toml}
   |
 1 | a = 42
   |     ^-- {message}
```

If colorization is enabled, ANSI escape sequences will be added for coloring.

When multiple `locs_and_msgs` are provided, they must be in the order of `const source_location&` followed by `const std::string&`, and the next pair in the same order, and so on.

#### Example: Multiple `source_location` and `std::string`

When multiple `source_location` and `std::string` pairs are provided, they are formatted as follows:


```cpp
source_location& loc0;
source_location& loc1;
source_location& loc2;

std::string msg0;
std::string msg1;
std::string msg2;

format_location(loc0, msg0,
                loc1, msg1,
                loc2, msg2);
```

```
 -> {filename0.toml}
   |
 1 | a = 42
   |     ^-- {message0}
   |
 -> {filename1.toml}
   |
 2 | b = 3.14
   |     ^-- {message1}
   |
 -> {filename2.toml}
   |
 3 | c = "foo"
   |     ^-- {message2}
```

# Related

- [error_info.hpp]({{<ref "error_info.md">}})
- [value.hpp]({{<ref "value.md">}})

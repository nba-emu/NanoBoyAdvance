+++
title = "error_info.hpp"
type  = "docs"
+++

# error_info.hpp

In `error_info.hpp`, definitions for `error_info` and functions to format it are provided.

# `toml::error_info`

```cpp
namespace toml
{
struct error_info
{
    error_info(std::string t, source_location l, std::string m, std::string s = "");
    error_info(std::string t, std::vector<std::pair<source_location, std::string>> l, std::string s = "");

    std::string const& title() const noexcept;
    std::string &      title()       noexcept;

    std::vector<std::pair<source_location, std::string>> const& locations() const noexcept;
    void add_locations(source_location loc, std::string msg) noexcept;

    std::string const& suffix() const noexcept;
    std::string &      suffix()       noexcept;
};

template<typename ... Ts>
error_info make_error_info(
    std::string title, source_location loc, std::string msg, Ts&& ... tail);

std::string format_error(const std::string& errkind, const error_info& err);
std::string format_error(const error_info& err);

template<typename ... Ts>
std::string format_error(std::string title,
                         source_location loc, std::string msg, Ts&& ... tail);

std::ostream& operator<<(std::ostream& os, const error_info& e);
}
```

## Member Functions

### Constructor (`title, loc, msg, suffix`)

Constructs `error_info` with specified `title`, location information `loc`, message `msg`, and optional `suffix`.

`suffix` defaults to empty.

### Constructor (`title, [{loc, msg}, ...], suffix`)

Constructs `error_info` with specified `title`, an array of location-message pairs `[{loc, msg}, ...]`, and optional `suffix`.

`suffix` defaults to empty.

### `std::string title()`

Returns the title of the error message.

### `std::vector<std::pair<source_location, std::string>> locations()`

Returns the list of locations where errors occurred along with their respective messages.

Multiple locations can be specified.

### `std::string suffix()`

Returns the suffix message to display at the end, providing hints or additional information.

## Non-Member Functions

### `make_error_info`

```cpp
template<typename ... Ts>
error_info make_error_info(
    std::string title, source_location loc, std::string msg, Ts&& ... tail);
```

Creates a new `error_info`.

Must be followed by a `msg` related to `source_location` or `basic_value`.

Overloads are added in [`value.hpp`]({{< ref "docs/reference/value#tomlmake_error_info" >}}) when passing `toml::basic_value` instead of `source_location`.

Possible to pass `suffix` at the end.

### `format_error`

Formats `error_info` as follows:

```
{title}
 --> {locations().at(0).first.file_name()}
   |
 1 | {locations().at(0).first.line()}
   |         ^-- {locations().at(0).second}
   |
 2 | {locations().at(1).first.line()}
   |         ^-- {locations().at(1).second}
{suffix}
```

If file names differ between two `source_location`, the file name is displayed again.

```cpp
std::string format_error(const std::string& errkind, const error_info& err);
std::string format_error(const error_info& err);
```

Formats `error_info`.

If `errkind` is not provided, a red-bold `[error]` prefix is added before `title`.

If `errkind` is provided (including an empty string), it replaces `[error]`.

```cpp
namespace toml
{
template<typename ... Ts>
std::string format_error(std::string title,
                         source_location loc, std::string msg, Ts&& ... tail);
} // toml
```

Returns a formatted string using `format_error` for `error_info` created with `make_error_info`.

Overloads are added in [`value.hpp`]({{< ref "docs/reference/value#tomlformat_error" >}}) when passing `toml::basic_value` instead of `source_location`.

### Stream Operator

```cpp
std::ostream& operator<<(std::ostream& os, const error_info& e);
```

Calls `format_error(e)` and outputs it.

# Related

- [color.hpp]({{< ref "color.md" >}})
- [parser.hpp]({{< ref "parser.md" >}})
- [source_location.hpp]({{< ref "source_location.md" >}})

+++
title = "color.hpp"
type  = "docs"
+++

# color.hpp

In `color.hpp`, functions related to colorizing error messages are defined.

Colors are specified using ANSI escape code.
In terminals or other output destinations that do not support ANSI escape code, the output may become difficult to read.

## Macros

### `TOML11_COLORIZE_ERROR_MESSAGE`

If this macro is defined during compilation (`-DTOML11_COLORIZE_ERROR_MESASGE`), error messages are colored by default.

If not defined, colors are not applied by default. You need to specify them using `toml::color::enable()`.

### `TOML11_USE_THREAD_LOCAL_COLORIZATION`

If this macro is defined during compilation (`-DTOML11_USE_THREAD_LOCAL_COLORIZATION`), the colorization flag becomes `thread_local`.
In this case, `toml::color::enable()` or `toml::color::disable()` will only affect the colorization flag in the thread that called it.
This means that if you want to use a different setting from the default, you will need to set it again when starting a new thread.
This makes `toml::color::enable()` and `toml::color::disable()` thread safe.

By default, the setting is global.
When it is global, if one thread executes `toml::color::enable()`, the error messages will be colored in all threads.
However, if one thread executes `enable()` or `disable()` while another executes `enable()`, `disable()` or `should_color()`, the result is undefined.

## Functions

### `enable()`

```cpp
namespace toml {
namespace color {
void enable();
} // color
} // toml
```

Enables colorization using ANSI escape code.

#### Example

```cpp
#include <toml.hpp>

int main()
{
    toml::color::enable(); // All subsequent errors will be colored.
    const auto input = toml::parse("input.toml");
    return 0;
}
```

### `disable()`

```cpp
namespace toml {
namespace color {
void disable();
} // color
} // toml
```

Disables colorization using ANSI escape code.

#### Example

```cpp
#include <toml.hpp>

int main()
{
    toml::color::disable(); // All subsequent errors will not be colored.
    const auto input = toml::parse("input.toml");
    return 0;
}
```

### `should_color()`

```cpp
namespace toml {
namespace color {
bool should_color();
} // color
} // toml
```

Returns `true` if colorization is enabled, `false` otherwise.

#### Example

```cpp
#include <toml.hpp>
#include <iomanip>
#include <iostream>

int main()
{
    std::cout << "colorized? : " << std::boolalpha << toml::color::should_color() << std::endl;
    return 0;
}
```

## Manipulators

```cpp
namespace toml {
namespace color {
std::ostream& reset  (std::ostream&);
std::ostream& bold   (std::ostream&);
std::ostream& grey   (std::ostream&);
std::ostream& gray   (std::ostream&);
std::ostream& red    (std::ostream&);
std::ostream& green  (std::ostream&);
std::ostream& yellow (std::ostream&);
std::ostream& blue   (std::ostream&);
std::ostream& magenta(std::ostream&);
std::ostream& cyan   (std::ostream&);
std::ostream& white  (std::ostream&);
} // color
} // toml
```

Colorizes the foreground with ANSI escape code.

#### Example

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    std::cout << toml::color::red << "red!" << toml::color::reset << std::endl;
    return 0;
}
```

# Related

- [error_info.hpp]({{<ref "error_info.md">}})

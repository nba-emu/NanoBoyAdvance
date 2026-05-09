+++
title = "error message"
type  = "docs"
weight = 30
+++

# Outputting Error Messages

`toml11` provides error messages that include location information within the file when using functions like `toml::parse`, `toml::get<T>/find<T>`, and `as_integer()`, among others.

For instance, if a syntax error in an integer is detected during parsing, an error message might look like this:

```
[error] bad integer: `_` must be surrounded by digits
 --> internal string at line 64 in file main.cpp
   |
 1 | a = 123__456
   |        ^-- invalid underscore
Hint: valid  : -42, 1_000, 1_2_3_4_5, 0xC0FFEE, 0b0010, 0o755
Hint: invalid: _42, 1__000, 0123
```

Or, if a type different from the one actually stored is requested:

```
[error] toml::value::as_string(): bad_cast to string
 --> input.toml
   |
 1 | a = 123_456
   |     ^^^^^^^-- the actual type is integer
```

`toml11` provides methods to create such error messages from `toml::value`.

By utilizing this feature, you can inform users not only about TOML syntax errors but also about application-specific errors. For example, if a negative number appears where only positive values are allowed, you can highlight the location within the TOML file to convey the error to the user.

## Creating Error Messages from `toml::value` Location Information

`toml::value` retains information about the location where it was parsed.

This information is encapsulated in `toml::source_location` and can be retrieved using `toml::value::location()`.

```cpp
const toml::value& a = input.at("a");
const toml::source_location src = a.location();
```

When a file is parsed with `toml::parse`, the TOML filename and line numbers are stored.

If parsed with `toml::parse_str`, the TOML filename is not available, but instead, the filename and line number of the C++ source code that called `toml::parse_str` are stored as the TOML filename. The first example on this page was output from `toml::parse_str`. Note the filename part.

For details, see the [reference]({{<ref "/docs/reference/source_location">}}).

You can build error information by passing a `toml::source_location` or `toml::value` and the associated error message to `toml::make_error_info`. Passing this to `toml::format_error` formats the error message into a `std::string`.

```cpp
const toml::value& a = input.at("a");
if(a.as_integer() < 0)
{
    const toml::error_info err = toml::make_error_info(
            "positive integer is required", // Error title
            a, "but got negative value"     // Message next to the value
        );
    std::cerr << toml::format_error(err) << std::endl;
}
```

This will output:

```
[error] positive integer is required
 --> input.toml
   |
 1 | a = -123456
   |     ^^^^^^^-- but got negative value
```

You can also add a supplementary message at the end. This part is not indented.

```cpp
const toml::value& a = input.at("a");
if(a.as_integer() < 0)
{
    const toml::error_info err = toml::make_error_info(
            "positive integer is required",      // Error title
            a, "but got negative value",         // Message next to the value
            "Hint: `a` means length of the data" // Supplementary message
        );
    std::cerr << toml::format_error(err) << std::endl;
}
```

This will output:

```
[error] positive integer is required
 --> input.toml
   |
 1 | a = -123456
   |     ^^^^^^^-- but got negative value
Hint: `a` means length of the data
```

{{<hint info>}}
The ability to output lines from the file using `toml::value` is because the parsed file is retained in memory as a string.

The parsed string is shared by `toml::value` via a `std::shared_ptr`. Copying it does not duplicate the entire file string. The file information is freed from memory when all `toml::value` instances constructed from parsing the file are destructed.

Therefore, when using this in an application, it is recommended to extract and store the required values during loading rather than directly storing `toml::value`.
{{</hint>}}

## Adding Colors to Strings

You can add color to error messages using ANSI escape codes.

If `TOML11_COLORIZE_ERROR_MESSAGE` is defined at compile time, the error messages output by toml11 will be colored by default.

If not, you can enable color for subsequent error messages by calling `toml::color::enable()`. Conversely, if you do not want colored output, for example, because the output is not to a console, call `toml::color::disable()`. You can check whether coloring is enabled at any point by calling `toml::color::should_color()`.

Additionally, while the error title, error message, and supplementary information are not colored by default, you can use manipulators from toml::color to add color to them.

```cpp
std::ostringstream oss;
oss << toml::color::red << "but got negative value";

const toml::error_info err = toml::make_error_info(
        "positive integer is required",      // Error title
        a, oss.str(),                        // Message next to the value
        "Hint: `a` means length of the data" // Supplementary message
    );
```

For more details, see the [reference]({{<ref "docs/reference/color">}}).

## Changing the Prefix of Error Messages from `[error]`

There may be different types of errors, and the default `[error]` prefix might not always be appropriate.

With `toml::format_error`, you can provide a `std::string` before `toml::error_info` to replace the `[error]` prefix.

For example:

```cpp
const toml::value& a = input.at("a");
if(a.as_integer() < 0)
{
    const toml::error_info err = toml::make_error_info(
            "positive integer is required", // Error title
            a, "but got negative value"     // Message next to the value
        );

    std::ostringstream prefix;
    prefix << toml::color::bold << toml::color::yellow << "[warn]";
    std::cerr << toml::format_error(prefix.str(), err) << std::endl;
    return 0;
}
else
{
    return a.as_integer();
}
```

This will output a warning starting with `[warn]`.

Additionally, you can create error messages without the `[error]` prefix by directly passing the components of `error_info` to `toml::format_error`.

```cpp
const toml::value& a = input.at("a");
if(a.as_integer() < 0)
{
    std::cerr << toml::format_error(
            "[warn] positive integer is required", // Error title
            a, "but got negative value"            // Message next to the value
        ) << std::endl;
    return 0;
}
else
{
    return a.as_integer()
}
```

## Creating Error Messages Referencing Multiple `toml::value`

In application settings, the range of permissible values might change based on previously read values.

In such cases, you may want to output the values causing the error simultaneously.

`toml::format_error` and `toml::make_error_info` can take multiple pairs of `toml::value` and their corresponding error messages as `std::string`.

```cpp
std::cerr << toml::format_error(
        "[error] invalid range",
        a, "minimum value is defined here",
        b, "maximum value is defined here",
        c, "and it exceeds the range"
    ) << std::endl;
```

You can also add supplementary information at the end.

```cpp
std::cerr << toml::format_error(
        "[error] invalid range",
        a, "minimum value is defined here",
        b, "maximum value is defined here",
        c, "and it exceeds the range",
        "Hint: all the values must be in the range [a, b)"
    ) << std::endl;
```

When passing `toml::value` or `toml::source_location`, an error message related to it must follow. If not, it will result in a very confusing compilation error.

+++
title = "changelog"
type  = "docs"
weight = 4
+++

# Change Log

# v4.4.0

## Added

- Add `toml::find_or_default()` (#280) (by Ken Matsui @ken-matsui)
- Add `erase` to `ordered_map` (#282) (#283) (by Sunlight @SunPodder)
- Add `bool basic_value::accessed() const` to detect whether the value has been accessed
  - enabled only if `TOML11_ENABLE_ACCESS_CHECK` is defined
- Add compare operators to `toml::spec`

## Fixed

- Use `is_void<T>` instead of `is_same<T, void>` (#281) (by Ken Matsui @ken-matsui)

## Changed

- Improve `toml::parse` performance by 2x
- Stop using deprecated Ubuntu 20 image on GitHub Actions

# v4.3.0

## Added

- Support `std::optional` as a template argument of `toml::find`
- Support multiple arguments `toml::visit(visitor, args...)`

## Fixed

- `toml::detail::region::last_` may be used uninitialized (#267) (#268) (by amatej @kontura)
- Fix use with CMake 3.21 and older (#271) (by Severin Leonhardt @SeverinLeonhardt)
- fix: prevent size_t-max length string allocation (#275) (#276) (by @hayt)
- update README.md (#277) (by somebody @oldoldtea) (by lz)
- Make parsing faster for very long line (#278)
- Avoid known problem in MSVC (#279)
- Check if `source_location::file_name()` is null before formatting

## Changed

- Update hugo-book theme
- Add MSVC 2017 to appveyor build

# v4.2.0

## Added

- Support `std::optional` members for `TOML11_DEFINE_CONVERSION_NON_INTRUSIVE` (by Ken Matsui)
- Make `thread_local` for `color_mode` optional (by Ken Matsui)
- add usage with CPM to README
- add explanation about `ordered_map` to README and update doc

## Fixed

- Manually cast file size to `std::streamsize` (by Pino Toscano)
- Typographical error in `table_format` output
- Format an empty array specified as array-of-table in one line
- Added a missing include file
- Fix typos in documentation template syntax (by Jack W)
- Fix `toml::find_or` for deeply nested tables

# v4.1.0

## Added

- support `std::get<std::u8string>`
- support `toml::value(std::u8string)`
- support `string_type = std::u8string`
- support `operator<<(std::ostream&, toml::value)`
- add `bool uppercase` to `toml::integer_format`
- support `template into_toml()` (by 萧迩珀)

## Fixed

- Fix not checking for `\r\n` when parsing line comments (by Andreas Keller)

## Changed

- speedup CI tests by multicore build

# v4.0.3

## Fixed

- remove default template argument from forward declaration in `toml_fwd.hpp`
- enable to call `toml::make_error_info` with multiple `toml::value`s
- enable to copy/move `toml::result` having `std::reference_wrapper`
- fix document generation error with the latest version of hugo
- fix several tiny errors that causes warning
- fix CMake compatibility warning

## Changed

add `-Werror` / `/WX` to build script
set MSVC warning level to `/W4`
add explanation of features to README

# v4.0.2

## Fixed

- check return value of fread in `parse(FILE*)`
- version macro defined in `toml11/version.hpp`
- update docs about compilation
- update docs about file open mode

## Changed
- use version macros defined in `toml11/version.hpp` as the project version in `CMakeLists.txt`

# v4.0.1

## Fixed

- Resolved naming conflict of `sematic_version::{major, minor}` with macros defined in `<sys/sysmacro.h>`
- Fixed the definition of `operator<<` in `discard_comments` (by Egor Pugin)
- Fixed the issue where the first blank line was not output in `format_location`
- Fixed the issue where error messages pointing to `source_location` referring to lines containing only newline characters were displayed in two lines
- Corrected links in the README
- Corrected the title of the README in `example/unicode`

## Added

- Configured CI to automatically update `single_include/toml.hpp` when changes are made to `main`

# Changes from v3 to v4

## Breaking Changes

### Changed `template` Parameters of `toml::basic_value`

In toml11 v3, `toml::basic_value` took separate template arguments for the comment container, table-type container, and array-type container.

```cpp
template<typename Comment,
         template<typename ...> class Table = std::unordered_map,
         template<typename ...> class Array = std::vector>
class basic_value;
```

However, this approach does not accommodate changes to types such as `integer_type`.

In toml11 v4, `toml::basic_value` now accepts a single `TypeConfig`, allowing for more types to be customized.

```cpp
template<typename TypeConfig>
class basic_value;
```

By default, the types stored in `toml::value` remain unchanged.

For more information on changing types, please refer to the
[`type_config`]({{< ref "/docs/reference/types.md">}}) documentation.

### Removed `std::initializer_list` Support for `toml::basic_value`

In toml11 v3, there was an overload for `toml::value` that accepted `std::initializer_list`. This allowed for more intuitive initialization of `toml::value` with arrays and tables.

```cpp
// toml11 v3
toml::value v{1,2,3,4,5};
toml::value v{ {"a", 42}, {"b", "foo"} };
```

However, this caused the following issues:

First, it was impossible to distinguish between a single-element array and a regular value, as it always became an array.

```cpp
// toml11 v3
toml::value v{1}; // Becomes [1,] instead of 1
```

With the widespread use of uniform initialization, this became very inconvenient.

Second, it was unclear whether the value represented a table with all string values or a nested array.

```cpp
// toml11 v3
toml::value v{ {"a", "foo"}, {"b", "bar"} };
// Could be either:
// {a = "foo", b = "bar"}
// [["a", "foo"], ["b", "bar"]]
```

These issues were difficult to resolve due to language specifications.

To avoid confusion, toml11 v4 has removed `std::initializer_list` support.

When initializing `toml::value` with an array, you must explicitly specify `toml::array`, and when initializing with a table, you must explicitly specify `toml::table`.

```cpp
// toml11 v4
toml::value v(toml::array{1,2,3,4,5});
toml::value v(toml::table{ {"a", 42}, {"b", "foo"} });

toml::value v{toml::array{1}}; // [1,]
toml::value v{1}               // 1

toml::value v{toml::table{{"a", "foo"}, {"b", "bar"}}};
toml::value v{toml::array{toml::array{"a", "foo"}, toml::array{"b", "bar"}}};
```

While this makes initializing `toml::value` with tables or arrays slightly less convenient, it ensures that the values will not become unpredictable by requiring explicit type information.

### Renamed `toml::basic_value::is_uninitialized()` to `is_empty()`

In toml11 v3, the function to check whether a `basic_value` was uninitialized was called `is_uninitialized`.

However, in toml11 v4, the library supports `null` values as an extension, allowing for the intentional construction of empty values.
Therefore, the function has been renamed to `is_empty` to reflect this change.

### Added Format Information and Removed `toml::string`

In toml11 v3, to retain information on whether a string was `basic` or `literal`, the library used a thin wrapper around `std::string` called `toml::string`.

```cpp
// toml11 v3
namespace toml
{
enum class string_t : std::uint8_t
{
    basic   = 0,
    literal = 1,
};

struct string
{
    string_t    kind;
    std::string str;
};
} // namespace toml
```

In toml11 v4, to accommodate more format information such as the numeric base or whether arrays should be multiline, every type now has an associated `xxx_format` type, which is stored alongside the value.

```cpp
// toml11 v4
enum class string_format : std::uint8_t
{
    basic             = 0,
    literal           = 1,
    multiline_basic   = 2,
    multiline_literal = 3
};

struct string_format_info
{
    string_format fmt = string_format::basic;
    bool start_with_newline    = false;
};
```

This change allows for more detailed format information to be preserved, ensuring that format specifics for numeric types, arrays, and tables are maintained even after parsing.

### Changed Arguments of `toml::format`

In toml11 v3, `toml::format` accepted values such as the precision and width of numeric types.

However, this approach did not allow for detailed formatting specifications, resulting in serialized files that did not match expectations.

In toml11 v4, each `toml::value` now carries its own format information, enabling more detailed formatting options to be preserved within the `toml::value` itself.

As a result, `toml::format` no longer accepts specific formatting values. Instead, it now only takes a `toml::spec`, which includes language feature flags used during formatting.

### Changed Member Functions of `toml::source_location`

In toml11 v3, the member types of `toml::source_location` were designed to handle only single lines.

In toml11 v4, the member types of `toml::source_location` are designed to handle multiple lines.

### Renamed `toml::format_underline` to `toml::format_location`

In toml11 v3, the function used to format location information from `toml::source_location` was called `toml::format_underline`.

To make the name clearer, it has been renamed to `toml::format_location`.

## Changed Arguments of `toml::format_error`

In toml11 v3, there was no class to represent error information, resulting in complex arguments for `toml::format_error`.

```cpp
template<typename C, template<typename ...> class T, template<typename ...> class A>
std::string format_error(const std::string& err_msg,
        const basic_value<C, T, A>& v, const std::string& comment,
        std::vector<std::string> hints = {},
        const bool colorize = TOML11_ERROR_MESSAGE_COLORIZED);

template<typename C, template<typename ...> class T, template<typename ...> class A>
inline std::string format_error(const std::string& err_msg,
        const toml::basic_value<C, T, A>& v1, const std::string& comment1,
        const toml::basic_value<C, T, A>& v2, const std::string& comment2,
        std::vector<std::string> hints = {},
        const bool colorize = TOML11_ERROR_MESSAGE_COLORIZED);

template<typename C, template<typename ...> class T, template<typename ...> class A>
inline std::string format_error(const std::string& err_msg,
        const toml::basic_value<C, T, A>& v1, const std::string& comment1,
        const toml::basic_value<C, T, A>& v2, const std::string& comment2,
        const toml::basic_value<C, T, A>& v3, const std::string& comment3,
        std::vector<std::string> hints = {},
        const bool colorize = TOML11_ERROR_MESSAGE_COLORIZED);
```

In toml11 v4, we have introduced `class error_info` and `make_error_info`, simplifying the arguments for `format_error`.

```cpp
std::string format_error(const error_info& err);
std::string format_error(const std::string& errkind, const error_info& err);

template<typename ... Ts>
std::string format_error(std::string title,
        source_location loc, std::string msg, Ts&& ... tail);

template<typename TC, typename ... Ts>
std::string format_error(std::string title,
        const basic_value<TC>& v, std::string msg, Ts&& ... tail);
```

### Changed Control of `toml::color`

In toml11 v3, to control whether to colorize the output, we used the manipulator `toml::colorize` in conjunction with `toml::color::enable/disable`.

The manipulator allowed us to decide whether to apply color to each stream,
but in v4, the frequency of using streams has decreased,
and issues such as the fact that `std::ios_base::xalloc` used internally is not thread-safe in C++11 have arisen.
Therefore, we have decided to use only `toml::color::enable/disable` and have removed `toml::colorize`.

## Non-Breaking Changes

### Added `parse_str`

In toml11 v3, there was no function to directly parse a string itself.
Therefore, when parsing a string, it was necessary to use `std::istringstream`.

To address this inconvenience, we have added `toml::parse_str`, allowing for direct parsing of strings.

### Added `try_parse`

In toml11 v3, when a parser encountered an error, it threw `toml::syntax_error`.

However, there are cases where you do not want to throw exceptions due to environments where exceptions cannot be thrown, or for reasons of performance.

In toml11 v4, we have implemented `toml::try_parse` using `toml::result`, which communicates parsing failures without throwing exceptions.

This doesn't mean exceptions are never thrown. Errors in the standard library being used, such as `std::bad_alloc` due to allocation failure from memory exhaustion, may still be thrown.

### Support for Parsing Byte Sequences

To allow for parsing TOML content obtained through means other than files, we have added `toml::parse` and `toml::try_parse` functions that accept `std::vector<unsigned char>`.

### Added `toml::spec`

In toml11 v3, all new features of the TOML language were incorporated, and features that were decided to be introduced in future versions of the TOML language were controlled by the macro `TOML11_USE_UNRELEASED_TOML_FEATURES`.

This was because, at the time of developing toml11 v3, the TOML language had not yet reached version 1.0.0, being at versions 0.4.0 to 0.5.0.

As not all users are familiar with the latest TOML language specification, displaying error messages based on an older language usage could confuse the entire community. Therefore, until reaching version 1.0.0, it was necessary to provide new language specifications as quickly as possible and encourage users to update.

However, the current TOML language specification is at version 1.0.0. Therefore, there was a need to be mindful of the choice to continue using version 1.0.0 even after TOML v1.1.0 was released.

To allow for more flexibility in selecting the TOML language specification, we introduced `toml::spec`, enabling the TOML language version to be changed at runtime.

Additionally, in `toml::spec`, flags are set for each language feature, allowing for the testing of specific language features only.

This mechanism is also used for TOML-specific language extensions in toml11 v4.

### Added Format Information

In toml11 v3, format information was not saved except for strings, and during serialization, only width and precision were considered.

However, this resulted in hexadecimal integers being serialized as decimal integers and no reliable way to ensure inline tables.

In toml11 v4, format information (`integer_format`, etc.) has been added to all TOML types, and it is now considered during parsing and serialization.

This allows for more detailed format information to be set for values, such as hexadecimal integers or inline tables.

### Changed to `preserve_comments` by Default

In toml11 v3, comments were not parsed by default and were also not serialized.

This was because comments were a later introduced feature and were being read through a special hack.

In toml11 v4, comments are parsed, preserved, and serialized by default.

Furthermore, the parser implementation has been significantly changed to ensure comments are parsed alongside other elements.

### Changed to Preserve Comments by Default

In toml11 v3, comments were neither parsed nor serialized by default. This was because comment support was a late addition, implemented through a special hack.

In toml11 v4, comments are now parsed, preserved, and serialized by default. The parser implementation has also been significantly revised so that comments are parsed just like any other element.

### Added `single_include/toml.hpp`

toml11 is a versatile library with different header files for various features to enhance development efficiency. However, this required a certain amount of effort to install.

Starting with toml11 v4, a `single_include/toml.hpp` file has been added, which combines all the header files in the correct order. This allows the library to be installed by simply copying a single file.

### Option to Use Precompiled Library

Due to the extensive use of templates in toml11, compile times have been long.

In toml11 v4, the number of precompilable functions has been increased, allowing them to be compiled ahead of time into a library. This is expected to reduce compile times when using the library in large-scale development projects.

### Reference Documentation

Previously, all features were documented in the README, with no detailed function definitions or reference materials available in Japanese.

In toml11 v4, reference documentation has been included, provided in both Japanese and English. However, since the library author is a native Japanese speaker, the Japanese content is considered primary. If there are discrepancies between the Japanese and English content, the Japanese version takes precedence.

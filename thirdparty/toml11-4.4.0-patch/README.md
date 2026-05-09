# toml11

[![Build Status on GitHub Actions](https://github.com/ToruNiina/toml11/workflows/build/badge.svg)](https://github.com/ToruNiina/toml11/actions)
[![Build status](https://ci.appveyor.com/api/projects/status/m2n08a926asvg5mg/branch/main?svg=true)](https://ci.appveyor.com/project/ToruNiina/toml11/branch/main)
[![Version](https://img.shields.io/github/release/ToruNiina/toml11.svg?style=flat)](https://github.com/ToruNiina/toml11/releases)
[![License](https://img.shields.io/github/license/ToruNiina/toml11.svg?style=flat)](LICENSE)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1209136.svg)](https://doi.org/10.5281/zenodo.1209136)

[日本語版](https://github.com/ToruNiina/toml11/blob/main/README_ja.md)

toml11 is a feature-rich TOML language library for C++11/14/17/20.

- It complies with [the latest TOML language specification](https://toml.io/en/v1.0.0).
- It passes all the standard TOML language [test cases](https://github.com/toml-lang/toml-test).
- It supports new features merged into the upcoming TOML version (v1.1.0).
- It provides clear error messages, including the location of the error.
- It parses and retains comments, associating them with corresponding values.
- It maintains formatting information such as hex integers and considers these during serialization.
- It provides exception-less parse function.
- It supports complex type conversions from TOML values.
- It allows customization of the types stored in `toml::value`.
- It provides some extensions not present in the TOML language standard.

## Example

```toml
# example.toml
title = "an example toml file"
nums  = [3, 1, 4, 1, 5] # pi!
```

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    // select TOML version at runtime (optional)
    auto data = toml::parse("example.toml", toml::spec::v(1,1,0));

    // find a value with the specified type from a table
    std::string title = toml::find<std::string>(data, "title");

    // convert the whole array into STL-like container automatically
    std::vector<int> nums = toml::find<std::vector<int>>(data, "nums");

    // access with STL-like manner
    if( ! data.contains("foo"))
    {
        data["foo"] = "bar";
    }
    if(data.at("nums").is_array())
    {
        data["nums"].as_array().push_back(9);
    }

    // check comments
    assert(data.at("nums").comments().at(0) == "# pi!");

    // pass a fallback
    std::string name = toml::find_or<std::string>(data, "name", "not found");

    // serialization considering format info
    data.at("nums").as_array_fmt().fmt = toml::array_format::multiline;
    data.at("nums").as_array_fmt().indent_type = toml::indent_char::space;
    data.at("nums").as_array_fmt().body_indent = 2;

    std::cout << toml::format(data) << std::endl;

    return 0;
}
```


For more details, please refer to the [documentation](https://toruniina.github.io/toml11/).

## Table of Contents

- [toml11](#toml11)
  - [Example](#example)
  - [Table of Contents](#table-of-contents)
  - [Integration](#integration)
    - [Single Include File](#single-include-file)
    - [git submodule](#git-submodule)
    - [CMake `FetchContent`](#cmake-fetchcontent)
    - [CMake Package Manager (CPM)](#cmake-package-manager-cpm)
    - [Install Using CMake](#install-using-cmake)
    - [Precompile Library](#precompile-library)
    - [Building Example](#building-example)
    - [Building Tests](#building-tests)
  - [Features](#features)
    - [Parsing a File](#parsing-a-file)
    - [finding a value](#finding-a-value)
    - [comments](#comments)
    - [error messages](#error-messages)
    - [serialization](#serialization)
    - [Configuring Types](#configuring-types)
  - [Examples](#examples)
  - [Changes from v3](#changes-from-v3)
    - [Breaking Changes](#breaking-changes)
    - [Added features](#added-features)
  - [Contributors](#contributors)
  - [Licensing terms](#licensing-terms)

## Integration

There are several ways to use toml11.

Here is a brief overview of each method. For more details, please refer to the [documentation](https://toruniina.github.io/toml11/docs/installation/).

### Single Include File

Copy `single_include/toml.hpp` to your preferred location and add it to your include path.

### git submodule

By adding toml11 as a subdirectory using `git submodule` (or any other way),
you can either add `toml11/include` to your include path or use `add_subdirectory(toml11)` in your CMake project.

### CMake `FetchContent`

Using `FetchContent`, you can automatically download it.

```cmake
include(FetchContent)
FetchContent_Declare(
  toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
  GIT_TAG        v4.4.0
)
FetchContent_MakeAvailable(toml11)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE toml11::toml11)
```

### CMake Package Manager (CPM)

After [adding cpm to your project](https://github.com/cpm-cmake/CPM.cmake?tab=readme-ov-file#adding-cpm), you can use toml11 by doing:

```cmake
include(cmake/CPM.cmake)

CPMAddPackage("gh:ToruNiina/toml11@4.4.0")

# OR

CPMAddPackage(
    NAME toml11
    GITHUB_REPOSITORY "ToruNiina/toml11"
    VERSION 4.4.0
    OPTIONS
    "TOML11_PRECOMPILE ON" # to pre-compile
    "TOML11_ENABLE_ACCESS_CHECK ON" # to use value.accessed()
    )

add_executable(main main.cpp)
target_link_libraries(main PUBLIC toml11::toml11)
```

### Install Using CMake

You can install toml11 using CMake with the following steps:

```console
$ git clone https://github.com/ToruNiina/toml11
$ cd toml11
$ git submodule update --init --recursive
$ cmake -B ./build/
$ cmake --build ./build/
$ cmake --install ./build/ --prefix /path/to/toml11
```

### Precompile Library

By setting `-DTOML11_PRECOMPILE=ON`, you can precompile some of the library functions.
In this case, the standard library features available will vary with the C++ version, and part of the interface will change.
Therefore, you need to specify `CMAKE_CXX_STANDARD`.

```console
$ cmake -B ./build/ -DTOML11_PRECOMPILE=ON -DCMAKE_CXX_STANDARD=11/14/17/20
$ cmake --build ./build/
```

When linking the library, use `target_link_libraries` in CMake

```cmake
target_link_libraries(your_target PUBLIC toml11::toml11)
```

or pass `-DTOML11_COMPILE_SOURCES` to the compiler.

### Building Example

To compile the examples in the `examples/` directory, set `-DTOML11_BUILD_EXAMPLES=ON`.

```console
$ cmake -B ./build/ -DTOML11_BUILD_EXAMPLES=ON
$ cmake --build ./build/
```

### Building Tests

To compile unit tests, set `-DTOML11_BUILD_TESTS=ON`.
Additionally, to compile the encoder and decoder for toml-test, set `-DTOML11_BUILD_TOML_TESTS=ON`.

```console
$ cmake -B ./build/ -DTOML11_BUILD_EXAMPLES=ON
$ cmake --build ./build/
```

## Features

Here is a brief overview of the features provided by toml11.

For more details, please refer to the [documentation](https://toruniina.github.io/toml11/docs/features/).

### Parsing a File

To parse a file, use `toml::parse`.

The overall type of the file is always a table.
However, since `toml::value` contains metadata such as comments and formatting information,
`toml::parse` returns a `toml::value` rather than a `toml::table`.

```cpp
const toml::value input = toml::parse("input.toml");
```

To parse a string directly, use `toml::parse_str`.

```cpp
const toml::value input = toml::parse_str("a = 42");
```

When parsing string literals, you can use the `""_toml` literal.

```cpp
using namespace toml::literals::toml_literals;
const toml::value lit = "a = 42"_toml;
```

`toml::parse`, `parse_str` and `_toml` literal throw a `toml::syntax_error` exception in case of a syntax error.

The error message obtained with `what()` will look like this:

```
[error] bad integer: `_` must be surrounded by digits
 --> internal string at line 64 in file main.cpp
   |
 1 | a = 123__456
   |        ^-- invalid underscore
Hint: valid  : -42, 1_000, 1_2_3_4_5, 0xC0FFEE, 0b0010, 0o755
Hint: invalid: _42, 1__000, 0123
```

Error messages can also be colorized by calling `toml::color::enable()`.

By using `toml::try_parse`, you can receive a `toml::result<toml::value, std::vector<toml::error_info>>` without throwing exceptions.

```cpp
const auto input = toml::try_parse("input.toml");
if(input.is_ok())
{
    std::cout << input.unwrap().at("a").as_integer() << std::endl;
}
```

Additionally, toml11 allows easy and precise control over the version of the TOML language being used.

You can enable specific new features from TOML v1.1.0 while using TOML v1.0.0.

```cpp
toml::spec s = toml::spec::v(1, 0, 0);
s.v1_1_0_allow_trailing_comma_in_inline_tables = true;

const toml::value input = toml::parse("input.toml");
```

Moreover, several language extensions not included in the TOML standard are available.

```cpp
toml::spec s = toml::spec::v(1, 0, 0);
s.ext_hex_float  = true; // this allows hexadecimal floating-point numbers
s.ext_null_value = true; // this allows `key = null` value
s.ext_num_suffix = true; // this allows numeric suffixes like `100_msec`
```

For more detail and reference of each feature, please refer to the [documentation](https://toruniina.github.io/toml11/docs/features/).

### finding a value

`toml::value` provides member functions for accessing values, such as `at()`, `is_xxx()`, and `as_xxx()`.

```cpp
const toml::value input = toml::parse("input.toml");
if(input.contains("a") && input.at("a").is_integer())
{
    std::cout << input.at("a").as_integer() << std::endl;
}
```

By using `toml::find`, you can perform type conversion and search simultaneously.

```cpp
const toml::value input = toml::parse("input.toml");
std::cout << toml::find<int>(input, "a") << std::endl;
```

If type conversion or value lookup fails, a `toml::type_error` is thrown. The error message will look like this:

```
[error] toml::value::as_string(): bad_cast to string
 --> input.toml
   |
 1 | a = 123_456
   |     ^^^^^^^-- the actual type is integer
```

You can access nested tables or arrays of tables in the same way.

```cpp
// [a]
// b = [
//   {c = 42},
//   {c = 54}
// ]
const toml::value input = toml::parse("input.toml");
std::cout << toml::find<int>(input, "a", "b", 1, "c") << std::endl;
```

Most STL containers and those with similar interfaces can be converted.

```cpp
// array = [3,1,4,1,5]
const toml::value input = toml::parse("input.toml");

const auto a1 = toml::find<std::vector<int>>(input, "array");
const auto a2 = toml::find<std::array<int, 5>>(input, "array");
const auto a3 = toml::find<std::deque<int>>(input, "array");
const auto a4 = toml::find<boost::container::small_vector<int, 8>>(input, "array");
```

You can perform advanced type conversions on complex TOML values.

```toml
mixed_array = [
    42,
    3.14,
    {a = "foo", b = "bar"}
]
```

```cpp
const toml::value input = toml::parse("input.toml");

const auto mixed = toml::find<
        std::tuple<int, double, std::map<std::string, std::string>>
    >(input, "mixed_array") << std::endl;
```

User-defined types can also be converted by using macros or defining some specific functions.

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(extlib::foo, a, b)

// ...

const auto input = R"(
  [foo]
  a = 42
  b = "bar"
)"_toml;
const extlib::foo f = toml::find<extlib::foo>(input, "foo");
```

Using `toml::find_or`, you can get a default value in case of failure.

```cpp
const toml::value input = toml::parse("input.toml");
std::cout << toml::find_or(input, "a", 6*9) << std::endl;
```

For more details, please refer to the [documentation](https://toruniina.github.io/toml11/docs/features/value/).

### comments

toml11 stores comments related to values within the value itself.

Comments related to a value include a series of comments written immediately before the value
and any comments written after the value without a newline in between.

```toml
# This is a comment for a.
# This is also a comment for a.
a = 42 # This is also a comment for a.

# This is separated by a newline, so it is not a comment for b.

# This is a comment for b.
b = "foo"
```

These comments are stored in `toml::value` with an interface similar to `std::vector<std::string>`.

```cpp
const toml::value input = toml::parse("input.toml");
std::cout << input.at("a").comments().size() << std::endl;
std::cout << input.at("a").comments().at(0) << std::endl;
```

For more details, please refer to the [documentation](https://toruniina.github.io/toml11/docs/features/value/#accessing-comments).

### error messages

One of the goals of toml11 is to provide clear and understandable error messages.

For parsing errors, you might see an error message like the following:

```
[error] bad integer: `_` must be surrounded by digits
 --> internal string at line 64 in file main.cpp
   |
 1 | a = 123__456
   |        ^-- invalid underscore
Hint: valid  : -42, 1_000, 1_2_3_4_5, 0xC0FFEE, 0b0010, 0o755
Hint: invalid: _42, 1__000, 0123
```

If you request a type different from the actual stored type, an error message like this will be displayed:

```
[error] toml::value::as_string(): bad_cast to string
 --> input.toml
   |
 1 | a = 123_456
   |     ^^^^^^^-- the actual type is integer
```

Such error messages can also be produced for user-specific algorithm that is not related to TOML syntax.

```cpp
const toml::value& a = input.at("a");
if(a.as_integer() < 0)
{
    const toml::error_info err = toml::make_error_info(
            "positive integer is required",
            a, "but got negative value"
        );
    std::cerr << toml::format_error(err) << std::endl;
}
```

For more details, please refer to the [documentation](https://toruniina.github.io/toml11/docs/features/error_message/).

### serialization

You can format a `toml::value` into a `std::string` using `toml::format`.

```cpp
const toml::value input = toml::parse("input.toml");
std::cout << toml::format(input) << std::endl;
```

`toml::format` references the formatting information, allowing you to change the formatting method.

```cpp
toml::value output(toml::table{ {"a", 0xDEADBEEF} });
output.at("a").as_integer_fmt().fmt = toml::integer_format::hex;
output.at("a").as_integer_fmt().spacer = 4; // position of `_`

std::cout << toml::format(input) << std::endl;
// a = 0xdead_beef
```

You can also specify the formatting for tables and arrays.

```cpp
toml::value output(toml::table{
  {"array-of-tables", toml::array{}},
  {"subtable", toml::table{}},
});

auto& aot = output.at("array-of-tables");
aot.as_array_fmt().fmt = toml::array_format::multiline; // one element per line
aot.as_array_fmt().body_indent    = 4;
aot.as_array_fmt().closing_indent = 2;

toml::value v1(toml::table{ {"a", 42}, {"b", 3.14} });
v1.as_table_fmt().fmt = toml::table_format::oneline;
aot.push_back(std::move(v1));

toml::value v2(toml::table{ {"a", 42}, {"b", 3.14} });
v2.as_table_fmt().fmt = toml::table_format::oneline;
aot.push_back(std::move(v2));

output.at("subtable").as_table_fmt().fmt = toml::table_format::dotted;
output.at("subtable")["a"] = 42;
output.at("subtable")["b"] = 3.14;

std::cout << toml::format(output) << std::endl;
// subtable.b = 3.14
// subtable.a = 42
// array-of-tables = [
//     {b = 3.14, a = 42},
//     {b = 3.14, a = 42},
//   ]
```

These settings are read during parsing and will be maintained as long as the value type does not change when modified.

For details on possible formatting specifications, please refer to the [documentation](https://toruniina.github.io/toml11/docs/features/serialize/).

### Configuring Types

Many types in `toml::value`, such as `integer_type`, `array_type`, `table_type`, etc, can be modified by changing the `type_config` type.

One commonly used example is an `ordered_map` that keeps the added order.
toml11 provides a`type_config` that changes `table_type` to `ordered_map` as `toml::ordered_type_config`.

```cpp
const toml::ordered_value input = toml::parse<toml::ordered_type_config>("input.toml");
```

Here, `toml::ordered_value` is an alias of `toml::basic_value<toml::ordered_type_config>`.

Note that, since `toml::value` uses `std::unordered_map`, once you convert it to `toml::value`, then the order of the values will be randomized.

For more details about how to implement `type_config` variant, please refer to the [documentation](https://toruniina.github.io/toml11/docs/features/configure_types/).

Also, refer to the [`examples` directory](https://github.com/ToruNiina/toml11/tree/main/examples) for complex use cases such as using multi-precision integers, changing containers, and normalizing Unicode.
Use these examples as references for implementing such configurations.

## Examples

The [`examples`](https://github.com/ToruNiina/toml11/tree/main/examples) directory provides various implementation examples in addition to type configurations.

- [boost_container](https://github.com/ToruNiina/toml11/tree/main/examples/boost_container)
  - This example shows how to use `boost::container` containers for `array_type` and `table_type`.
- [boost_multiprecision](https://github.com/ToruNiina/toml11/tree/main/examples/boost_multiprecision)
  - This example demonstrates the use of `boost::multiprecision` multi-precision numeric types for `integer_type` and `floating_type`.
- [parse_file](https://github.com/ToruNiina/toml11/tree/main/examples/parse_file)
  - This example includes type conversion implementations, covering slightly more complex cases. The corresponding TOML file is included.
- [reflect](https://github.com/ToruNiina/toml11/tree/main/examples/reflect)
  - This example shows self-type conversion using boost-ext/reflect for user-defined types.
- [unicode](https://github.com/ToruNiina/toml11/tree/main/examples/unicode)
  - This example demonstrates normalizing Unicode strings when searching for keys using uni-algo.

## Changes from v3

toml11 v4 introduces several breaking changes.

Efforts have been made to minimize the need for changes for those using simple functionality.
However, if you were utilizing advanced features, some adjustments may be necessary.

Nevertheless, we believe that the added or streamlined features will provide enhanced convenience in return.

### Breaking Changes

- Changed branch from `master` to `main`.
- Changed template arguments of `toml::basic_value`.
- Removed `toml::string` and explicitly store formatting information of all the values.
- Modified arguments of `toml::format` to accommodate formatting information.
- Changed default arguments of `toml::parse` to take `toml::spec` for specifying TOML version information.
- Updated the interface of `toml::source_location` to accommodate multiline regions.
- Modified arguments of `toml::format_error`.
- Renamed `toml::format_underline` to `toml::format_location`.
- Unified control method of `toml::color` to `toml::color::enable/disable()`.

### Added features

- Added `toml::parse_str`.
- Added `toml::try_parse`.
- Added support for parsing byte sequences.
- Added formatting information to `toml::value`.
- Changed to saving comments in `toml::value` by default.
- Added `single_include/toml.hpp`.
- Enabled to use as a precompiled library.

## Contributors

I appreciate the help of the contributors who introduced the great feature to this library.

- Guillaume Fraux (@Luthaf)
  - Windows support and CI on Appvayor
  - Intel Compiler support
- Quentin Khan (@xaxousis)
  - Found & Fixed a bug around ODR
  - Improved error messages for invalid keys to show the location where the parser fails
- Petr Beneš (@wbenny)
  - Fixed warnings on MSVC
- Ivan Shynkarenka (@chronoxor)
  - Fixed Visual Studio 2019 warnings
  - Fix compilation error in `<filesystem>` with MinGW
- Khoi Dinh Trinh (@khoitd1997)
  - Fixed warnings while type conversion
- @KerstinKeller
  - Added installation script to CMake
- J.C. Moyer (@jcmoyer)
  - Fixed an example code in the documentation
- Jt Freeman (@blockparty-sh)
  - Fixed feature test macro around `localtime_s`
  - Suppress warnings in Debug mode
- OGAWA Kenichi (@kenichiice)
  - Suppress warnings on intel compiler
  - Fix include path in README
- Jordan Williams (@jwillikers)
  - Fixed clang range-loop-analysis warnings
  - Fixed feature test macro to suppress -Wundef
  - Use cache variables in CMakeLists.txt
  - Automate test set fetching, update and refactor CMakeLists.txt
- Scott McCaskill
  - Parse 9 digits (nanoseconds) of fractional seconds in a `local_time`
- Shu Wang (@halfelf)
  - fix "Finding a value in an array" example in README
- @maass-tv and @SeverinLeonhardt
  - Fix MSVC warning C4866
- Mohammed Alyousef (@MoAlyousef)
  - Made testing optional in CMake
- Alex Merry (@amerry)
  - Add missing include files
- sneakypete81 (@sneakypete81)
  - Fix typo in error message
- Oliver Kahrmann (@founderio)
  - Fix missing filename in error message if parsed file is empty
- Karl Nilsson (@karl-nilsson)
  - Fix many spelling errors
- ohdarling88 (@ohdarling)
  - Fix a bug in a constructor of serializer
- estshorter (@estshorter)
  - Fix MSVC warning C26478
- Philip Top (@phlptp)
  - Improve checking standard library feature availability check
- Louis Marascio (@marascio)
  - Fix free-nonheap-object warning
- Axel Huebl (@ax3l)
  - Make installation optional if the library is embedded
- Ken Matsui (@ken-matsui)
  - Support user-defined error message prefix
  - Support dynamic color mode
  - Support `std::optional` members for `TOML11_DEFINE_CONVERSION_NON_INTRUSIVE`
  - Make `thread_local` for `color_mode` optional
  - Add `toml::find_or_default`
  - Fix static assertions in success that checks if specified type is void
- Giel van Schijndel (@muggenhor)
  - Remove needless copy in `parse` function
- Lukáš Hrázký (@lukash)
  - Add a `parse(FILE *)` interface and improve file-related error messages
- spiderman idog (@spiderman-idog)
  - Fix typo in README
- Jajauma's GitHub (@Jajauma)
  - Avoid possible lexer truncation warnings
- Moritz Klammler (@ctcmkl)
  - Many patches in (#200) including:
  - Improve CMake scripts, build process, and test file handling
  - Detect error when `discard_comments` is accessed
  - And more.
- Chris White (@cxw42)
  - Fix address-sanitizer error when parsing literal strings having invalid UTF-8 characters
  - Fix function name in error messages
- offa (@offa)
  - Update checkout action to v3
  - Update Required CMake version
  - Cleanup old CI settings
- Sergey Vidyuk (@VestniK)
  - Fix for case when vector iterator is raw pointer
- Kfir Gollan (@kfirgollan)
  - Add installation example with checkinstall and cmake
- Martin Tournoij (@arp242)
  - Escape control characters in keys
- @DavidKorczynski
  - Add fuzzing test based on ClusterFuzzLite
- Esonhugh Skyworship (@Esonhugh)
  - Fix function signature of `strerror_r` on macos
- Alberto (@0X1A)
  - Fix issues with CMake package configuration when used with vcpkg
- Egor Pugin (@egorpugin)
  - Fix incorrect operator<<() argument type that gives build error
- Andreas Keller (@andreaskeller96)
  - Fix not checking for \r\n when parsing line comments
- 萧迩珀 (@CDK6182CHR)
  - Support template into_toml members
- Pino Toscano (@pinotree)
  - Suppress warnings by manually cast file size to `std::streamsize`
- Jack W (@jackwil1)
  - Fix typos in documentation template syntax
- amatej (@kontura)
  - Fix: `toml::detail::region::last_` may be used uninitialized
- Severin Leonhardt (@SeverinLeonhardt)
  - Fix use with CMake 3.21 and older
- hayt (@hayt)
  - fix: prevent size_t-max length string allocation
- somebody (@oldoldtea), (lz)
  - Update README for better ToC, fixing example code
- Sunlight (@SunPodder)
  - Add `erase(...)` function to `ordered_map`

## Licensing terms

This product is licensed under the terms of the [MIT License](LICENSE).

- Copyright (c) 2017-2024 Toru Niina

All rights reserved.

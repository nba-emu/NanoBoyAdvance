+++
title = "parsing files"
type  = "docs"
weight = 10
+++

# Parsing Files and Strings

In toml11, you can parse files, strings, and byte arrays using `toml::parse` or `toml::try_parse`.

Upon success, these functions return a `toml::value`.
Although the parsed file is always a table, the return type is not `toml::table`.
This is because `toml::value` contains metadata about the file, whereas `toml::table` is merely an alias for `std::unordered_map<std::string, toml::value>`.
To include metadata, a `toml::value` is returned instead of a `toml::table`.
The `toml::value` corresponding to the root of the file will always hold a `table_type`.

## Parsing Files

To parse files, use either
[`toml::parse`]({{< ref "docs/reference/parser#parse" >}})
or
[`toml::try_parse`]({{< ref "docs/reference/parser#try_parse" >}}).

### `toml::parse`

#### Specifying the Filename with `std::string`

[`toml::parse`]({{< ref "docs/reference/parser#parse" >}})
accepts a filename as a string, opens the file, and parses it.

The following sample code parses a file named `input.toml`, extracts the `title` variable as a string, and prints it.

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    const toml::value input = toml::parse("input.toml");
    std::cout << input.at("title").as_string() << std::endl;
    return 0;
}
```

#### Specifying a File with `std::filesystem::path`

[`toml::parse`]({{< ref "docs/reference/parser#parse" >}}) can accept a `std::filesystem::path`.

This requires C++17 or later, as it relies on the `<filesystem>` support.

#### Specifying an Input Stream with `std::istream`

[`toml::parse`]({{< ref "docs/reference/parser#parse" >}}) can also accept an `std::istream`.

Open a stream in binary mode by passing `std::ios::binary` to avoid inconsistency between the file size and the number of characters due to automatic conversion of newline characters by the standard library.

Without the filename information, error messages will display `"unknown file"`. To avoid this, you can pass the filename as a `std::string` in the second argument when using `std::istream`.

You can use streams other than `std::ifstream`, such as `std::istringstream`. Note that the entire content is readable at the time of the call.

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    std::string filename("input.toml");
    std::ifstream ifs(filename);
    const toml::value input = toml::parse(ifs, filename);
    std::cout << input.at("title").as_string() << std::endl;
    return 0;
}
```

#### Specifying a File with `FILE*`

[`toml::parse`]({{< ref "docs/reference/parser#parse" >}}) can also accept a `FILE*`.

Open a stream in binary mode by passing `"rb"` to avoid inconsistency between the file size and the number of characters due to automatic conversion of newline characters by the standard library.

As with `std::istream`, you need to provide the filename as a string in the second argument.

When passing a `FILE*`, if the file read fails, `errno` will be reported.

#### Error Handling

[`toml::parse`]({{< ref "docs/reference/parser#parse" >}}) throws a [`toml::syntax_error`]({{< ref "docs/reference/parser#syntax_error" >}}) if it encounters a syntax error.

[`toml::syntax_error`]({{< ref "docs/reference/parser#syntax_error" >}}) is derived from [`toml::exception`]({{< ref "docs/reference/exception" >}}), which in turn is derived from `std::exception`.

Therefore, you can use the `what()` member function to retrieve the error message from a [`toml::syntax_error`]({{< ref "docs/reference/parser#syntax_error" >}}).

Additionally, [`toml::syntax_error`]({{< ref "docs/reference/parser#syntax_error" >}}) contains a [`std::vector<toml::error_info>`]({{< ref "docs/reference/error_info" >}}), which can be accessed using the `errors()` member function.

`toml::parse` attempts to recover from minor errors and report multiple errors whenever possible. While it can often recover from simple errors like number format issues, errors within arrays or tables might not be recoverable and may lead to multiple similar error reports. If you find the error messages redundant, you can use only the `front()` of the `std::vector<toml::error_info>` to get a message about the most critical issue.

```cpp
#include <toml.hpp>

int main()
{
    try {
        const toml::value input = toml::parse("input.toml");
        std::cout << input.at("title").as_string() << std::endl;
    } catch(const toml::syntax_error& err) {
        // report all the errors
        std::cerr << err.what() << std::endl;

        // report the first error only
        std::cerr << err.errors().front() << std::endl;
    }
}
```

### `toml::try_parse`

While `toml::parse` throws an exception on failure, `toml::try_parse` does not throw exceptions when it fails.

Instead, its return type is [`toml::result<toml::value, std::vector<toml::error_info>>`]({{<ref "docs/reference/result#result">}}).

The [`result`]({{<ref "docs/reference/result#result">}}) type holds either a success value or a failure value, similar to Rust's `Result` or Haskell's `Either`.

```cpp
#include <toml.hpp>

int main()
{
    const auto parse_result = toml::try_parse("input.toml");
    if(parse_result.is_ok())
    {
        std::cout << parse_result.unwrap().at("title").as_string() << std::endl;
    }
    else
    {
        std::cerr << parse_result.unwrap_err().at(0) << std::endl;
    }
    return 0;
}
```

To check which value the [`result`]({{<ref "docs/reference/result#result">}}) type holds, use the `is_ok()` and `is_err()` functions. The success or failure value can be retrieved using `unwrap()` and `unwrap_err()`, respectively. If `unwrap` fails, it throws a `bad_result_access` exception. Using the `as_ok()` and `as_err()` functions does not throw exceptions on failure, but results in undefined behavior.

{{<hint warning>}}

Although `try_parse` does not throw `syntax_error` or `file_io_error`, it returns the same `toml::error_info` as the failure type in the `result`. However, it is not entirely exception-free.

If an internal standard library error occurs, such as `std::bad_alloc` when a `vector` fails to allocate memory, `toml::try_parse` does not catch this and will let it propagate. Thus, exceptions originating from the standard library may still be thrown.

{{</hint>}}

## Parsing Strings

### `toml::parse_str`

[`toml::parse_str`]({{<ref "docs/reference/parser#parse_str">}}) accepts the string to be parsed directly, instead of a filename.

For the part of the error message that corresponds to the TOML file's name, if the `std::source_location` equivalent compiler extension is available, the name and line number of the C++ file that called `parse_str` will be used instead.

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    const toml::value input = toml::parse_str("title = \"parse_str\"");
    std::cout << input.at("title").as_string() << std::endl;
    return 0;
}
```

### `toml::try_parse_str`

[`toml::try_parse_str`]({{<ref "docs/reference/parser#try_parse_str">}}) also takes the string to be parsed directly, similar to `parse_str`. Like `try_parse`, it uses [`toml::result`]({{<ref "docs/reference/result#result">}}) to report errors.

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    const auto parse_result = toml::try_parse_str("title = \"parse_str\"");
    if(parse_result.is_ok())
    {
        std::cout << parse_result.unwrap().at("title").as_string() << std::endl;
    }
    else
    {
        std::cerr << parse_result.unwrap_err().at(0) << std::endl;
    }
    return 0;
}
```

## Parsing Byte Arrays

It is also possible to parse byte arrays instead of files.

Since the byte arrays must be encoded in UTF-8, `unsigned char` is used.

### `toml::parse(std::vector<unsigned char>)`

The behavior is the same as [`toml::parse`]({{<ref "docs/reference/parser#parse">}}).

When parsing byte arrays, a `filename` is required.

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    std::vector<unsigned char> bytes{/* ... */};
    const toml::value input = toml::parse(bytes, "internal bytes");
    std::cout << input.at("title").as_string() << std::endl;
    return 0;
}
```

### `toml::try_parse(std::vector<unsigned char>)`

The behavior is the same as [`toml::try_parse`]({{<ref "docs/reference/parser#try_parse">}}).

When parsing byte arrays, a `filename` is required.

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    std::vector<unsigned char> bytes{/* ... */};
    const auto parse_result = toml::try_parse(bytes, "internal bytes");
    if(parse_result.is_ok())
    {
        std::cout << parse_result.unwrap().at("title").as_string() << std::endl;
    }
    else
    {
        std::cerr << parse_result.unwrap_err().at(0) << std::endl;
    }
    return 0;
}
```


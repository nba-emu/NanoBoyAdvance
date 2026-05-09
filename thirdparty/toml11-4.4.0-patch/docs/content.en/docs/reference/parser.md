+++
title = "parser.hpp"
type  = "docs"
+++

# parser.hpp

Defines functions for parsing files or strings and the exceptions they use.

While `parse` throws an exception on failure, `try_parse` returns error information.

# `parse`

Parses the content of a given file and returns a `toml::basic_value`.

In case of failure, `toml::syntax_error` is thrown.

The type information of `basic_value` is provided by a `template`, and the TOML language version is specified by `toml::spec`.

### `parse(std::string filename, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
basic_value<TC>
parse(std::string fname,
      spec s = spec::default_version());
}
```

Parses the content of the given filename.

If reading the file fails, `toml::file_io_error` is thrown.

If parsing fails, `toml::syntax_error` is thrown.

### `parse(const char (&)[N] filename, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config, std::size_t N>
basic_value<TC>
parse(const char (&fname)[N],
      spec s = spec::default_version());
}
```

Parses the content of the given filename from a string literal.

If reading the file fails, `toml::file_io_error` is thrown.

If parsing fails, `toml::syntax_error` is thrown.

### `parse(std::filesystem::path, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
basic_value<TC>
parse(const std::filesystem::path& fpath,
      spec s = spec::default_version());
}
```

This is defined only if `<filesystem>` is available.

Parses the content of the file at the given file path.

If reading the file fails, `toml::file_io_error` is thrown.

If parsing fails, `toml::syntax_error` is thrown.

### `parse(std::istream&, std::string filename, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
basic_value<TC>
parse(std::istream& is,
      std::string fname = "unknown file",
      spec s = spec::default_version());
}
```

Parses the content of the given `std::istream&`.

Open a stream in binary mode by passing `std::ios::binary` to avoid inconsistency between the file size and the number of characters due to automatic conversion of newline characters by the standard library.

The filename information is taken as the third argument. If the filename is not provided, it defaults to `"unknown file"`.

### `parse(FILE*, std::string filename, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
result<basic_value<TC>, std::vector<error_info>>
parse(FILE* fp,
      std::string filename,
      spec s = spec::default_version());
}
```

Parses the content of the file pointed to by `FILE*`.

Open a stream in binary mode by passing `"rb"` to avoid inconsistency between the file size and the number of characters due to automatic conversion of newline characters by the standard library.

If reading the file fails, `file_io_error` containing `errno` is thrown.

If parsing fails, `syntax_error` is thrown.

### `parse(std::vector<unsigned char>, std::string filename, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
basic_value<TC>
parse(std::vector<unsigned char> content,
      std::string filename,
      spec s = spec::default_version());
}
```

Parses the byte sequence as a TOML file.

If parsing fails, `toml::syntax_error` is thrown.

# `parse_str`

### `parse_str(std::string, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
basic_value<TC>
parse_str(std::string content,
          spec s = spec::default_version(),
          cxx::source_location loc = cxx::source_location::current());
}
```

Parses a string as a TOML file.

In case of failure, `toml::syntax_error` is thrown.

The type information of `basic_value` is provided by a `template`, and the TOML language version is specified by `toml::spec`.

You generally don't need to manually set the third argument, `cxx::source_location`.
If `std::source_location`, `std::experimental::source_location`, or `__builtin_FILE` is available,
the location information where `parse_str` was called will be stored.

# `try_parse`

Parses the contents of the given file and returns a `toml::basic_value` if successful, or a `std::vector<toml::error_info>` if it fails.

The type information of `basic_value` is specified by `template`, and the version of the TOML language is specified by `toml::spec`.

{{< hint warning >}}

Unlike `parse`, `try_parse` does not throw exceptions defined in toml11 such as `syntax_error`.
However, please note that exceptions thrown by the standard library will still propagate.

For instance, errors occurring internally within `std::ifstream` or memory exhaustion in `std::vector` will throw exceptions.

{{< /hint >}}

### `try_parse(std::string filename, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
result<basic_value<TC>, std::vector<error_info>>
try_parse(std::string fname,
          spec s = spec::default_version());
}
```

Takes a file name and parses its content.

If parsing fails, a `result` holding the error type `std::vector<error_info>` is returned.

If successful, a `result` holding a `basic_value` is returned.

### `try_parse(const char (&)[N] filename, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config, std::size_t N>
result<basic_value<TC>, std::vector<error_info>>
try_parse(const char (&fname)[N],
          spec s = spec::default_version());
}
```

Takes a string literal as a file name and parses its content.

If parsing fails, a `result` holding the error type `std::vector<error_info>` is returned.

If successful, a `result` holding a `basic_value` is returned.

### `try_parse(std::filesystem::path, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
result<basic_value<TC>, std::vector<error_info>>
try_parse(const std::filesystem::path& fpath,
          spec s = spec::default_version());
}
```

Takes a file path and parses its content.

If parsing fails, a `result` holding the error type `std::vector<error_info>` is returned.

If successful, a `result` holding a `basic_value` is returned.

### `try_parse(std::istream&, std::string filename, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
result<basic_value<TC>, std::vector<error_info>>
try_parse(std::istream& is,
          std::string fname = "unknown file",
          spec s = spec::default_version());
}
```

Takes a `std::istream&` and parses its content.

Open a stream in binary mode by passing `std::ios::binary` to avoid inconsistency between the file size and the number of characters due to automatic conversion of newline characters by the standard library.

The file name information is taken as the second argument. If a file name is not provided, it defaults to `"unknown file"`.

If parsing fails, a `result` holding the error type `std::vector<error_info>` is returned.

If successful, a `result` holding a `basic_value` is returned.

### `try_parse(FILE*, std::string filename, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
result<basic_value<TC>, std::vector<error_info>>
try_parse(FILE* fp,
          std::string filename,
          spec s = spec::default_version());
}
```

Takes a `FILE*` and parses its content.

Open a stream in binary mode by passing `"rb"` to avoid inconsistency between the file size and the number of characters due to automatic conversion of newline characters by the standard library.

If parsing fails, a `result` holding the error type `std::vector<error_info>` is returned.

If successful, a `result` holding a `basic_value` is returned.

### `try_parse(std::vector<unsigned char>, std::string filename, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
result<basic_value<TC>, std::vector<error_info>>
try_parse(std::vector<unsigned char> content,
          std::string filename,
          spec s = spec::default_version());
}
```

Takes a byte array and parses its content as a TOML file.

If parsing fails, a `result` holding the error type `std::vector<error_info>` is returned.

If successful, a `result` holding a `basic_value` is returned.

# `try_parse_str`

### `try_parse_str(std::string, toml::spec)`

```cpp
namespace toml
{
template<typename TC = type_config>
result<basic_value<TC>, std::vector<error_info>>
try_parse_str(std::string content,
              spec s = spec::default_version(),
              cxx::source_location loc = cxx::source_location::current());
}
```

Parses a string as a TOML file, returning a `toml::basic_value` if successful, or a `std::vector<toml::error_info>` if it fails.

Unlike `parse_str`, it does not throw `syntax_error`, but instead returns error information as the failure type of the `result`.

If `std::source_location`, `std::experimental::source_location`, or `__builtin_FILE` is available, it will record the location information.

Typically, you do not need to manually set the third argument `cxx::source_location`. If any of `std::source_location`, `std::experimental::source_location`, or `__builtin_FILE` are available, the information of the location where `parse_str` was called will be saved as the location information.

{{< hint warning >}}

Unlike `parse`, `try_parse` does not throw exceptions defined in toml11 such as `syntax_error`. However, please note that exceptions thrown by the standard library will still propagate.

For instance, errors occurring internally within `std::ifstream` or memory exhaustion in `std::vector` will throw exceptions.

{{< /hint >}}

# `syntax_error`

```cpp
namespace toml
{
struct syntax_error final : public ::toml::exception
{
  public:
    syntax_error(std::string what_arg, std::vector<error_info> err);
    ~syntax_error() noexcept override = default;

    const char* what() const noexcept override;
    std::vector<error_info> const& errors() const noexcept
};
}
```

An exception thrown when a syntax error is detected in TOML.

It is thrown by `parse` but not by `try_parse`.

# `file_io_error`

```cpp
namespace toml
{
struct file_io_error final : public ::toml::exception
{
  public:
    file_io_error(const std::string& msg, const std::string& fname);
    file_io_error(int errnum, const std::string& msg, const std::string& fname);
    ~file_io_error() noexcept override = default;

    const char* what() const noexcept override;

    bool has_errno() const noexcept;
    int  get_errno() const noexcept;
};
}
```

An exception thrown when reading the contents of a file fails.

When using `FILE*` to read a file, `errno` is set.

### `has_errno`

If `std::ifstream` fails, `errno` is not set.

In this case, `has_errno` returns `false`.

### `get_errno`

Particularly when passing a `FILE*`, retrieves the value of `errno`.

If `has_errno` is `false`, it returns `0`.

# Related

- [error_info.hpp]({{<ref "error_info.md">}})
- [result.hpp]({{<ref "result.md">}})
- [spec.hpp]({{<ref "spec.md">}})
- [value.hpp]({{<ref "value.md">}})

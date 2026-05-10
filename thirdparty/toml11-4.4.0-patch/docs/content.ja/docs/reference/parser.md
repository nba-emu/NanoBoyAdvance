+++
title = "parser.hpp"
type  = "docs"
+++

# parser.hpp

ファイルまたは文字列をパースする関数と、それが用いる例外を定義します。

`parse`は失敗した場合に例外を送出しますが、`try_parse`はエラー情報を返します。

# `parse`

与えられたファイルの内容をパースし、`toml::basic_value`を返します。

失敗した場合は`toml::syntax_error`が送出されます。

`basic_value`の持つ型情報は`template`で、TOML言語のバージョンは`toml::spec`で指定します。

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

ファイル名を受け取って、その内容をパースします。

ファイルの読み込みに失敗した場合、`file_io_error`が送出されます。

パースに失敗した場合、`syntax_error`が送出されます。

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

文字列リテラルを受け取って、そのファイルの内容をパースします。

ファイルの読み込みに失敗した場合、`file_io_error`が送出されます。

パースに失敗した場合、`syntax_error`が送出されます。

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

`<filesystem>`が利用可能な場合のみ定義されます。

ファイルパスを受け取って、そのファイルの内容をパースします。

ファイルの読み込みに失敗した場合、`file_io_error`が送出されます。

パースに失敗した場合、`syntax_error`が送出されます。

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

`std::istream&`を受け取ってその内容をパースします。

標準ライブラリが改行文字を自動変換することによるファイルサイズと文字数との不整合を避けるため、
`std::ios::binary`を使ってバイナリモードで開いてください。

ファイル名の情報は第三引数で受け取ります。ファイル名が渡されなかった場合、`"unknown file"`になります。

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

`FILE*`が指すファイルを読み込んでパースします。

標準ライブラリが改行文字を自動変換することによるファイルサイズと文字数との不整合を避けるため、
`fopen`には`"rb"`などを渡してバイナリモードで開いてください。

ファイルの読み込みに失敗した場合、`errno`が含まれた`file_io_error`が送出されます。

パースに失敗した場合、`syntax_error`が送出されます。

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

バイト列をTOMLファイルとしてパースします。

パースに失敗した場合、`syntax_error`が送出されます。

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

文字列をTOMLファイルとしてパースします。

失敗した場合は`toml::syntax_error`が送出されます。

`basic_value`の持つ型情報は`template`で、TOML言語のバージョンは`toml::spec`で指定します。

第三引数の`cxx::source_location`を手動で設定する必要は通常ありません。
`std::source_location`, `std::experimental::source_location`, `__builtin_FILE`のいずれかが利用可能な場合、
`parse_str`が呼ばれた地点の情報が位置情報として保存されます。

# `try_parse`

与えられたファイルの内容をパースし、成功した場合は`toml::basic_value`を、失敗した場合は`std::vector<toml::error_info>`を返します。

`basic_value`の持つ型情報は`template`で、TOML言語のバージョンは`toml::spec`で指定します。

{{< hint warning >}}

`try_parse`は`parse`と異なり`syntax_error`などのtoml11で定義された例外は投げませんが、
標準ライブラリから送出される例外はそのまま送出されることに注意してください。

例えば、`std::ifstream`の内部で起きたエラーや、`std::vector`でのメモリ枯渇などは例外を送出します。

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

ファイル名を受け取って、その内容をパースします。

パースに失敗した場合、エラー型である`std::vector<error_info>`を持つ`result`が返されます。

成功した場合、`basic_value`を持つ`result`が返されます。


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

文字列リテラルをファイル名として受け取って、その内容をパースします。

パースに失敗した場合、エラー型である`std::vector<error_info>`を持つ`result`が返されます。

成功した場合、`basic_value`を持つ`result`が返されます。

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

ファイルパスを受け取って、その内容をパースします。

パースに失敗した場合、エラー型である`std::vector<error_info>`を持つ`result`が返されます。

成功した場合、`basic_value`を持つ`result`が返されます。

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

`std::istream&`を受け取ってその内容をパースします。

標準ライブラリが改行文字を自動変換することによるファイルサイズと文字数との不整合を避けるため、
`std::ios::binary`を使ってバイナリモードで開いてください。

ファイル名の情報は第二引数で受け取ります。ファイル名が渡されなかった場合、`"unknown file"`になります。

パースに失敗した場合、エラー型である`std::vector<error_info>`を持つ`result`が返されます。

成功した場合、`basic_value`を持つ`result`が返されます。

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

`FILE*`を受け取って、そのファイルの内容をパースします。

標準ライブラリが改行文字を自動変換することによるファイルサイズと文字数との不整合を避けるため、
`fopen`には`"rb"`などを渡してバイナリモードで開いてください。

パースに失敗した場合、エラー型である`std::vector<error_info>`を持つ`result`が返されます。

成功した場合、`basic_value`を持つ`result`が返されます。

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

バイト列を受け取って、その内容をTOMLファイルとしてパースします。

パースに失敗した場合、エラー型である`std::vector<error_info>`を持つ`result`が返されます。

成功した場合、`basic_value`を持つ`result`が返されます。

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

文字列をTOMLファイルとしてパースし、成功した場合は`toml::basic_value`を、失敗した場合は`std::vector<toml::error_info>`を返します。

`parse_str`と異なり、`syntax_error`を送出せず、エラー情報を`result`の失敗型として返します。

`std::source_location`, `std::experimental::source_location`, `__builtin_FILE`のどれかが利用可能な場合、それを位置情報に記録します。

第三引数の`cxx::source_location`を手動で設定する必要は通常ありません。
`std::source_location`, `std::experimental::source_location`, `__builtin_FILE`のいずれかが利用可能な場合、
`parse_str`が呼ばれた地点の情報が位置情報として保存されます。

{{< hint warning >}}

`try_parse`は`parse`と異なり`syntax_error`などのtoml11で定義された例外は投げませんが、
標準ライブラリから送出される例外はそのまま送出されることに注意してください。

例えば、`std::ifstream`の内部で起きたエラーや、`std::vector`でのメモリ枯渇などは例外を送出します。

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

TOML言語の文法エラーが発見された場合に送出される例外です。

`parse`からは送出されますが、`try_parse`からは送出されません。

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

ファイルの内容を読むのに失敗した場合に送出される例外です。

`FILE*`を使ってファイルを読み込んだ場合は`errno`が設定されます。

### `has_errno`

`std::ifstream`が失敗した場合は`errno`は設定されません。

このとき、`has_errno`は`false`になります。

### `get_errno`

特に`FILE*`を渡していた場合に、`errno`の値を取得します。

`has_errno`が`false`の場合は`0`となります。

# 関連項目

- [error_info.hpp]({{<ref "error_info.md">}})
- [result.hpp]({{<ref "result.md">}})
- [spec.hpp]({{<ref "spec.md">}})
- [value.hpp]({{<ref "value.md">}})

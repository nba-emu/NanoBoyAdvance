+++
title = "source_location.hpp"
type  = "docs"
+++

# source_location.hpp

`source_location.hpp`では、TOMLファイル内のある領域を指すクラスが定義されます。

このクラスは、エラーメッセージで問題の箇所を指摘するために使われます。

# `toml::source_location`

`source_location`は、TOMLファイル内のある領域を指すクラスです。

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

## メンバ関数
 
### コンストラクタ

```cpp
explicit source_location(/* implementation-defined */);
```

`toml::source_location`を`toml::parse`または`_toml`リテラル以外で構築することはできません。

### `is_ok()`

```cpp
bool is_ok() const noexcept;
```

`source_location`が有効な値を保持している場合、`true`を、そうでない場合`false`を返します。

`toml::parse`や`_toml`リテラル以外から構築した`toml::value`の`location()`の結果は、指す対象がないため、`is_ok`は`false`を返します。

### `length()`

```cpp
std::size_t length() const noexcept;
```

`source_location`が指す領域の長さを返します。

有効な値を保持していない場合、`0`を返します。

### `first_line_number()`

```cpp
std::size_t first_line_number() const noexcept;
```

`source_location`が指す領域の最初の行の行番号を返します。

有効な値を保持していない場合、`1`を返します。

### `first_column_number()`

```cpp
std::size_t first_column_number() const noexcept;
```

`source_location`が指す領域の最初の列の列番号を返します。

有効な値を保持していない場合、`1`を返します。

### `last_line_number()`

```cpp
std::size_t last_line_number() const noexcept;
```

`source_location`が指す領域の最後の行の行番号を返します。

有効な値を保持していない場合、`1`を返します。

### `last_column_number()`

```cpp
std::size_t last_column_number() const noexcept;
```

`source_location`が指す領域の最後の列の列番号を返します。

有効な値を保持していない場合、`1`を返します。

### `file_name()`

```cpp
std::string const& file_name() const noexcept;
```

`source_location`が指す領域を含むファイルのファイル名を返します。

有効な値を保持していない場合、`"unknown file"`を返します。

### `num_lines()`

```cpp
std::size_t num_lines() const noexcept;
```

`source_location`が指す領域の行数を返します。

有効な値を保持していない場合、`0`を返します。

### `first_line()`

```cpp
std::string const& first_line() const;
```

`source_location`が指す領域の最初の行を返します。

有効な値を保持していない場合、`std::out_of_range`例外が送出されます。

### `last_line()`

```cpp
std::string const& last_line() const;
```

`source_location`が指す領域の最後の行を返します。

有効な値を保持していない場合、`std::out_of_range`例外が送出されます。

### `lines()`

```cpp
std::vector<std::string> const& lines() const noexcept;
```

`source_location`が指す領域の全ての行を返します。

有効な値を保持していない場合、空の`std::vector`への参照を返します。

## 非メンバ関数
 
### `format_location`

```cpp
template<typename ... Ts>
std::string format_location(const source_location& loc, const std::string& msg, const Ts& ... locs_and_msgs);
```

`source_location`が指す箇所と、それについてのメッセージを以下のようにフォーマットします。

```
 -> {filename.toml}
   |
 1 | a = 42
   |     ^-- {message}
```

この時、色付けがONになっている場合、ANSIエスケープシーケンスによって色情報が追加されます。

locs_and_msgsが複数個ある場合、それらは`const source_location&`と`const std::string&`の順である必要があります。

#### 例：複数の`source_location`と`std::string`

複数の`source_location`と`std::string`を渡した場合、以下のようにフォーマットされます。

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

# 関連項目

- [error_info.hpp]({{<ref "error_info.md">}})
- [value.hpp]({{<ref "value.md">}})

+++
title = "error_info.hpp"
type  = "docs"
+++

# error_info.hpp

`error_info.hpp`では、`error_info`と、それをフォーマットする関数が定義されます。

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

## メンバ関数

### コンストラクタ(`title, loc, msg, suffix`)

指定されたタイトル、位置情報、メッセージ、suffixから`error_info`を構築します。

`suffix`はデフォルトで空です。

### コンストラクタ(`title, [{loc, msg}, ...], suffix`)

指定されたタイトル、位置情報とメッセージの配列、そして`suffix`から`error_info`を構築します。

`suffix`はデフォルトで空です。

## メンバ関数

### `std::string title()`

エラーメッセージのタイトルです。

### `std::vector<std::pair<source_location, std::string>> locations()`

エラーの発生した位置とそれに関するメッセージです。

複数指定可能です。

### `std::string suffix()`

最後に表示するメッセージです。ヒントや補足を表示します。

## 非メンバ関数

### `make_error_info`

```cpp
template<typename ... Ts>
error_info make_error_info(
    std::string title, source_location loc, std::string msg, Ts&& ... tail);
```

新しく`error_info`を構築します。

`source_location`または`basic_value`の後には、それに関する`msg`が続かなければなりません。

[`value.hpp`]({{<ref "docs/reference/value#tomlmake_error_info">}})
では、 `source_location` の代わりに `toml::basic_value` を渡した際のオーバーロードが追加されます。

末尾には`suffix`を渡すことが可能です。

### `format_error`

`error_info` を以下のようにフォーマットします。

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

二つの `source_location` のファイル名が異なる場合は、ファイル名が再度表示されます。

```cpp
std::string format_error(const std::string& errkind, const error_info& err);
std::string format_error(const error_info& err);
```

`error_info`をフォーマットします。

`errkind`が与えられなかった場合、赤色太字の`[error]`が`title`の前につけ足されます。

`errkind`が与えられた場合（空文字列の場合も含みます）、それが`[error]`の代わりに表示されます。


```cpp
namespace toml
{
template<typename ... Ts>
std::string format_error(std::string title,
                         source_location loc, std::string msg, Ts&& ... tail);
} // toml
```

`make_error_info` を使って作成した `error_info` を `format_error` でフォーマットした文字列を返します。

[`value.hpp`]({{<ref "docs/reference/value#tomlformat_error">}})
では、 `source_location` の代わりに `toml::basic_value` を渡した際のオーバーロードが追加されます。

### ストリーム演算子

```cpp
std::ostream& operator<<(std::ostream& os, const error_info& e);
```

`format_error(e)`を呼び出し、それを出力します。


# 関連項目

- [color.hpp]({{<ref "color.md">}})
- [parser.hpp]({{<ref "parser.md">}})
- [source_location.hpp]({{<ref "source_location.md">}})

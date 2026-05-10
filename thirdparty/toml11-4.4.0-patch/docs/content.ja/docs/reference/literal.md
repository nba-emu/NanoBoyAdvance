+++
title = "literal.hpp"
type  = "docs"
+++

# literal.hpp

`literal.hpp`では、`_toml`リテラルが定義されます。

`_toml`リテラルは、文字列リテラルをパースして`toml::value`に変換します。

```cpp
namespace toml
{
inline namespace literals
{
inline namespace toml_literals
{
toml::value operator"" _toml(const char* str, std::size_t len);
toml::value operator"" _toml(const char8_t* str, std::size_t len); // C++20以降
} // toml_literals
} // literals
} // toml
```

## 自由関数

### `operator"" _toml(char)`

```cpp
toml::value operator"" _toml(const char* str, std::size_t len);
```

文字列リテラルをパースして`toml::value`に変換します。

通常のTOMLファイルの場合、`toml::parse`と同等の処理が行われます。

```cpp
const auto v1 = "a = 'foo'"_toml; // v1: {a = 'foo'}
```

改行を含む場合、生文字列リテラルが便利です。

```cpp
const auto v1 = R"(
    a = 42
    b = "foo"
)"_toml;
```

値が単体で書かれていた場合、その値になります。

```cpp
const auto v2 = "'foo'"_toml;     // v2: 'foo'
```

TOMLは数値のみからなるキーを許可しています。
`[1]`のように、テーブル定義と配列の区別がつかない場合、テーブル定義が優先されます。

配列として解釈させるには、trailing commaを使用してください。

```cpp
const auto v3 = "[1]"_toml;  // v3: {1 = {}}
const auto v4 = "[1,]"_toml; // v4: [1,]
```

### `operator"" _toml(char8_t)`

`char8_t`が利用可能な場合に定義されます。引数の型のほかに違いはありません。

# 例

```cpp
#include <toml.hpp>
int main()
{
    using namespace toml::literals::toml_literals;
    const auto v = "a = \"foo\""_toml;
    assert(v.at("a").as_string() == "foo");
    return 0;
}
```


+++
title = "toml literal"
type  = "docs"
weight = 60
+++

# `_toml`リテラル

`""_toml`リテラルによって、TOMLファイルをその場でフォーマットできます。

```cpp
#include <toml.hpp>

int main()
{
    using namespace toml::literals::toml_literals;

    const auto v = "a = 42"_toml;

    assert(v.at("a").as_integer() == 42);

    return 0;
}
```

改行を含む場合、生文字列リテラルが便利です。

```cpp
#include <toml.hpp>

int main()
{
    using namespace toml::literals::toml_literals;

    const auto v = R"(
        a = 42
        b = "foo"
    )"_toml;

    assert(v.at("a").as_integer() == 42);
    assert(v.at("b").as_string()  == "foo");

    return 0;
}
```

値が単体で書かれていた場合、その値が返されます。

```cpp
#include <toml.hpp>

int main()
{
    using namespace toml::literals::toml_literals;

    const auto a = "42"_toml;
    const auto b = "12:34:56"_toml;

    assert(v.at("a").as_integer() == 42);
    assert(v.at("b").as_local_time().hour   == 12);
    assert(v.at("b").as_local_time().minute == 34);
    assert(v.at("b").as_local_time().second == 56);

    return 0;
}
```

TOMLは数値のみからなるキーを許可しています。
よって、`[1]`はテーブル名として合法です。

`[1]`のようにテーブル定義と配列の区別がつかない場合、テーブル定義が優先されます。

配列として解釈させるには、trailing commaを使用してください。

```cpp
#include <toml.hpp>

int main()
{
    using namespace toml::literals::toml_literals;

    const auto t = "[1]"_toml;  // {1 = {}}
    const auto a = "[1,]"_toml; // [1,]

    assert(t.is_table());
    assert(t.at("1").is_table());

    assert(a.is_array());
    assert(a.at(0).as_integer() == 1);

    return 0;
}
```

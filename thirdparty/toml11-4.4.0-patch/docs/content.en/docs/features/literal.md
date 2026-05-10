+++
title = "toml literal"
type  = "docs"
weight = 60
+++

# `_toml` Literal

With the `""_toml` literal, you can format TOML files inline.

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

When including line breaks, raw string literals come in handy.

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

If a value is written on its own, that value is returned.

```cpp
#include <toml.hpp>

int main()
{
    using namespace toml::literals::toml_literals;

    const auto a = "42"_toml;
    const auto b = "12:34:56"_toml;

    assert(a.as_integer() == 42);
    assert(b.as_local_time().hour   == 12);
    assert(b.as_local_time().minute == 34);
    assert(b.as_local_time().second == 56);

    return 0;
}
```

TOML allows keys consisting solely of numbers. Therefore, `[1]` is a valid table name.

When there's ambiguity between table definitions and arrays, table definitions take precedence.

To interpret as an array, please use a trailing comma.

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

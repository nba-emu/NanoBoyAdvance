+++
title = "conversion.hpp"
type  = "docs"
+++

# conversion.hpp

`toml::get`や`toml::find`でユーザー定義型をサポートするための変換関数を自動定義するマクロを提供します。

```cpp
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(NAME, ...)
```

## 例

```cpp
namespace foo
{
struct Foo
{
    std::string s;
    double      d;
    int         i;
};
} // foo

TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(foo::Foo, s, d, i)
```

# 関連項目

- [from.hpp]({{<ref "from.md">}})
- [into.hpp]({{<ref "into.md">}})

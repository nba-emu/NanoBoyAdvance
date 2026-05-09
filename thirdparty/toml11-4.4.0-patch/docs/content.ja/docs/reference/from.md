+++
title = "from.hpp"
type  = "docs"
+++

# from.hpp

`toml::get`や`toml::find`で使用する、`toml::value`からの変換を定義する構造体です。

メンバ関数に`from_toml`を追加することによっても同じ機能を追加できますが、メンバ関数を追加できないクラスに対しては`from<T>`を使用してください。

このファイルでは特定の実装は提供しません。使用する際に、この構造体を特殊化してください。

```cpp
namespace toml
{

template<typename T>
struct from;

} // toml
```

## 例

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib

#include <toml11/from.hpp>

namespace toml
{
template<>
struct from<extlib::foo>
{
    template<typename TC>
    static extlib::foo from_toml(const toml::basic_value<TC>& v)
    {
        return extlib::foo{toml::find<int>(v, "a"), toml::find<std::string>(v, "b")};
    }
};
} // toml
```

# 関連項目

- [conversion.hpp]({{<ref "conversion.md">}})
- [into.hpp]({{<ref "into.md">}})

+++
title = "into.hpp"
type  = "docs"
+++

# into.hpp

`toml::value`のコンストラクタで使用する、ユーザー定義型からの変換を定義する構造体です。

メンバ関数に`into_toml`を追加することによっても同じ機能を追加できますが、メンバ関数を追加できないクラスに対しては`into<T>`を使用してください。

このファイルでは特定の実装は提供しません。使用する際に、この構造体を特殊化してください。

```cpp
namespace toml
{

template<typename T>
struct into;

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

#include <toml11/into.hpp>

namespace toml
{
template<>
struct into<extlib::foo>
{
    template<typename TC>
    static toml::basic_value<TC> into_toml(const extlib::foo& f)
    {
        using value_type = toml::basic_value<TC>;
        using table_type = typename value_type::table_type;
        return value_type(table_type{{"a", f.a}, {"b", f.b}});
    }
};
} // toml
```

# 関連項目

- [conversion.hpp]({{<ref "conversion.md">}})
- [from.hpp]({{<ref "from.md">}})

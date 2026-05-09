+++
title = "visit.hpp"
type  = "docs"
+++

# visit.hpp

`visit.hpp`では、`toml::visit`が定義されます。

# `toml::visit`

## 関数

```cpp
namespace toml
{

template<typename Visitor, typename ... Args>
/* visitor にArgsの中身を渡した際の返り値 */
visit(Visitor&& visitor, Args&& ... args);

} // toml
```

`basic_value<TC>`が保持している型に対応する`Visitor`のオーバーロードを呼び出し、その結果を返します。

#### 条件

`Visitor`は、`basic_value<TC>`が保持している型のどれに対しても呼び出し可能な関数または関数オブジェクトでなければなりません。

また、それぞれのオーバーロードで返り値は同じであることが要求されます。

#### 例

```cpp
#include <toml.hpp>
#include <iostream>

struct type_name_of
{
    std::string operator()(const toml::value::boolean_type        &) const {return "boolean";}
    std::string operator()(const toml::value::integer_type        &) const {return "integer";}
    std::string operator()(const toml::value::floating_type       &) const {return "floating";}
    std::string operator()(const toml::value::string_type         &) const {return "string";}
    std::string operator()(const toml::value::local_time_type     &) const {return "local_time";}
    std::string operator()(const toml::value::local_date_type     &) const {return "local_date";}
    std::string operator()(const toml::value::local_datetime_type &) const {return "local_datetime";}
    std::string operator()(const toml::value::offset_datetime_type&) const {return "offset_datetime";}
    std::string operator()(const toml::value::array_type          &) const {return "array";}
    std::string operator()(const toml::value::table_type          &) const {return "table";}
};

int main()
{
    toml::value v(3.14);
    std::cout << toml::visit(type_name_of{}, v) << std::endl; // floating
    return 0;
}
```

# 関連項目

- [value.hpp]({{<ref "value.md">}})

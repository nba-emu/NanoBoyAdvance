+++
title = "value_t.hpp"
type  = "docs"
+++

# value_t.hpp

型情報を表す列挙型です。

# `value_t`

`value_t`は、`toml::value`が持つ型情報を扱う際に使用します。

```cpp
namespace toml
{
enum class value_t : std::uint8_t
{
    empty           =  0,
    boolean         =  1,
    integer         =  2,
    floating        =  3,
    string          =  4,
    offset_datetime =  5,
    local_datetime  =  6,
    local_date      =  7,
    local_time      =  8,
    array           =  9,
    table           = 10
};

std::ostream& operator<<(std::ostream& os, value_t t);
std::string to_string(value_t t);
} // toml
```

## 非メンバ関数

### ストリーム演算子

```cpp
std::ostream& operator<<(std::ostream& os, value_t t);
```

`value_t`の値を文字列化してストリームへ出力します。

### `to_string`

```cpp
std::string to_string(value_t t);
```

`value_t`の値を文字列化して返します。


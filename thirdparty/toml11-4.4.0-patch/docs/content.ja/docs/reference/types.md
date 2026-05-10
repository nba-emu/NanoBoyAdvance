+++
title = "types.hpp"
type  = "docs"
+++

# types.hpp

型情報を与えるクラスが定義されます。

# `type_config`

`type_config`は、`toml::basic_value`に与えられるパラメータをまとめた型です。

`toml::basic_value<T>`内で異なる型を使用する場合、これを別に定義して渡します。
記載のある要素は全て必須の要素です。

通常のストリーム演算子を使用できない数値型を使用する場合、`read_int`、`read_float`に相当するものを定義し、置き換えてください。

```cpp
namespace toml
{
struct type_config
{
    using comment_type  = preserve_comments;

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    using table_type = std::unordered_map<K, T>;

    static result<integer_type, error_info>
    parse_int(const std::string& str, const source_location src, const std::uint8_t base);

    static result<floating_type, error_info>
    parse_float(const std::string& str, const source_location src, const bool is_hex);
};

using value = basic_value<type_config>;
using table = typename value::table_type;
using array = typename value::array_type;

} // toml
```

## `static` メンバ関数

### `parse_int(str, src, base)`

```cpp
static result<integer_type, error_info>
parse_int(const std::string& str, const source_location src, const std::uint8_t base);
```

通常のストリーム演算子などを使用できない型を`integer_type`として使用する場合、この関数を実装してください。

`str`には、prefix、（`0x`などの場合）leading zero、underscoreが取り除かれた文字列が渡されます。

`src`には、その文字列が定義されていた箇所を指す`source_location`が渡されます。

`base`には、`10`, `2`, `8`, `16`のいずれかが渡されます。

### `parse_float(str, src, is_hex)`

```cpp
static result<floating_type, error_info>
parse_float(const std::string& str, const source_location src, const bool is_hex);
```

通常のストリーム演算子などを使用できない型を`floating_type`として使用する場合、この関数を実装してください。

`str`には、prefix、leading zero、underscoreが取り除かれた文字列が渡されます。

`src`には、その文字列が定義されていた箇所を指す`source_location`が渡されます。

`is_hex`には、フォーマットが`hexfloat`であるかどうかが渡されます。`hexfloat`拡張を使用しない場合は使われないので、実装する必要はありません。

`hexfloat`拡張に関しては、[spec.hpp]({{<ref "spec.md">}})を参照してください。

## 非メンバ関数

### `read_int`

```cpp
template<typename T>
result<T, error_info>
read_int(const std::string& str, const source_location src, const std::uint8_t base);
```

デフォルトで使用される関数です。`std::istringstream`を使用してパースします。

`operator>>`と`std::hex`等のマニピュレータ、`std::numeric_limits<T>`が定義されている場合（`boost::multiprecision`など）、特に変更なしにこれを使用できます。

### `read_float`

```cpp
template<typename T>
result<T, error_info>
read_float(const std::string& str, const source_location src, const bool is_hex);
```

デフォルトで使用される関数です。decimalの場合は`std::istringstream`を、hexfloatの場合は`sscanf()`を使用してパースします。

`double`、`float`に対応しています。

それ以外の型の場合、`operator>>`が定義されていて、かつ`hex`を使用しないなら、これを使用できます。

# `ordered_type_config`

`ordered_type_config`は、`toml::type_config`のテーブル型を`toml::ordered_map`に変更したものです。
また、`toml::ordered_value`エイリアスを定義します。

そのほかに`type_config`との違いはありません。

```cpp
namespace toml
{
struct ordered_type_config
{
    using comment_type  = preserve_comments;

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    using table_type = ordered_map<K, T>;

    static result<integer_type, error_info>
    parse_int(const std::string& str, const source_location src, const std::uint8_t base)
    {
        return read_int<integer_type>(str, src, base);
    }
    static result<floating_type, error_info>
    parse_float(const std::string& str, const source_location src, const bool is_hex)
    {
        return read_float<floating_type>(str, src, is_hex);
    }
};

using ordered_value = basic_value<ordered_type_config>;
using ordered_table = typename ordered_value::table_type;
using ordered_array = typename ordered_value::array_type;

} // toml
```


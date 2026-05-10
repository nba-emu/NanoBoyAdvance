+++
title = "configuring types"
type  = "docs"
weight = 50
+++

# 型をカスタマイズする

`toml::value` は `integer_type` として `std::int64_t` を、
`table_type` として `std::unordered_map<key_type, value_type>` を使用します。

しかし、場合によっては `boost::multiprecision::int128_t` や、 `std::map` を使用したい場合もあります。

そのため、 `toml::value` は `template` 引数を取って格納する型を変えられるように実装されています。

`std::string` が実際には `std::basic_string<char, std::char_traits<char>, std::allocator<char>>` の
エイリアスであるように、 `toml::value` は実際には `toml::basic_value<toml::type_config>` のエイリアスです。

ここでは、 `toml::type_config` が持つ型と、異なる `config` 型を定義する方法を説明します。

## `type_config`

`type_config` は、以下のメンバ型と`static`メンバ関数を持つクラスです。

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
}
```

`toml::basic_value<TypeConfig>` は、格納する `boolean_type` を `TypeConfig::boolean_type`、
格納する `integer_type` を `TypeConfig::integer_type` 、のようにして定義しています。

また、 `array_type` は `TypeConfig::array_type<toml::basic_value<TypeConfig>>` 、
`table_type` は `TypeConfig::table_type<key_type, toml::basic_value<TypeConfig>>` と
定義されます。

これらのメンバ型とメンバ関数を定義したクラスを `toml::basic_value` に
渡すことで、その `toml::basic_value` が持つ型を変更できます。

`parse_int` と `parse_float` は、数値型を独自実装のものに変更した際にパース方法を提供するための関数です。
これらの関数には `0x` などのprefixと桁区切りの `_` が取り除かれた文字列、
例えば `123456` や `DEADBEEF` が渡されます。
`base` には `10`, `16`, `8`, `2` のいずれかが渡されます。
これらを使って `integer_type` と `floating_type` をパース出来るような関数を渡してください。

デフォルト実装として、`toml::read_int` と `toml::read_float` が提供されています。

```cpp
static result<integer_type, error_info>
parse_int(const std::string& str, const source_location src, const std::uint8_t base)
{
    return toml::read_int<integer_type>(str, src, base);
}

static result<floating_type, error_info>
parse_float(const std::string& str, const source_location src, const bool is_hex)
{
    return toml::read_float<floating_type>(str, src, is_hex);
}
```

`read_int` は `istream` を使用し、16進と8進の場合は `std::hex` と
`std::oct` を使用します。2進の場合は掛け算と足し算で実装されています。
これらをサポートしている型であれば、 `read_int` をそのまま使用できます。

`read_float` は `istream` を使用します。
16進浮動小数点数は `double` と `float` の場合しかサポートされておらず、
それ以外の型のときに呼ばれると常にパースエラーを返す実装になっているので、
もし浮動小数点数型をこれら以外の型にし、かつ `hexfloat` を使用する場合は、
それを実装してください。 `hexfloat` を使用しないのであれば、実装する必要はありません。

## テーブル内の値の順序を維持する

デフォルトの `toml::type_config` の他に、 `toml::ordered_type_config` が提供されています。
これは、 `table_type` を [ordered_map]({{< ref "docs/reference/ordered_map" >}}) に変更したものです。

これを使用したものを `toml::ordered_value` 、その配列型とテーブル型のエイリアスを
`toml::ordered_array` と `toml::ordered_table` と定義しています。

`toml::parse(...)` を `toml::parse<toml::ordered_type_config>(...)` として呼び出すことで、
`toml::ordered_value` を使用することができます。

```cpp
#include <toml.hpp>

int main()
{
    toml::ordered_value input = toml::parse<toml::ordered_type_config>("example.toml");
    std::cout << toml::format(input) << std::endl;
    return 0;
}
```

## コメントを保存しない

`type_config` は `comment_type` でコメントを保存するコンテナを定義しています。

コメントに特に情報がなく、パースせずに捨ててしまっていい場合は、 `comment_type` に
`toml::discard_comments` を指定してください。

```cpp
struct wo_comment_config
{
    using comment_type  = toml::discard_comments; // XXX

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    using table_type = std::unordered_map<K, T>;

    static result<integer_type, error_info>
    parse_int(const std::string& str, const source_location src, const std::uint8_t base)
    {
        return toml::read_int<integer_type>(str, src, base);
    }

    static result<floating_type, error_info>
    parse_float(const std::string& str, const source_location src, const bool is_hex)
    {
        return toml::read_float<floating_type>(str, src, is_hex);
    }
};
```

## 配列に`std::vector`以外のコンテナを使用する

TOML配列の実装に`vector`以外のコンテナ（例：`std::deque`）を使用するには、
`array_type` を以下のように変更してください。

また、テーブル型のコンテナに `unordered_map` 以外のコンテナ（例：`std::map`）を使用するには、
`table_type` を以下のように変更してください。

```cpp
struct deque_map_config
{
    using comment_type  = toml::preserve_comments;

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::deque<T>; // XXX
    template<typename K, typename T>
    using table_type = std::map<K, T>; // XXX

    static result<integer_type, error_info>
    parse_int(const std::string& str, const source_location src, const std::uint8_t base)
    {
        return toml::read_int<integer_type>(str, src, base);
    }

    static result<floating_type, error_info>
    parse_float(const std::string& str, const source_location src, const bool is_hex)
    {
        return toml::read_float<floating_type>(str, src, is_hex);
    }
};
```

## 数値型に `boost::multiprecision` を使用する

`boost::multiprecision::cpp_int` と `boost::multiprecision::cpp_bin_float_oct`
を使用することで、より幅の広い整数型とより精度の良い浮動小数点数型を使用することができます。

これらの型はストリーム演算子を実装しているため、デフォルト実装の `read_int` と
`read_float` をそのまま使用できます。

```cpp
struct large_num_config
{
    using comment_type  = toml::preserve_comments;

    using boolean_type  = bool;
    using integer_type  = boost::multiprecision::cpp_int;
    using floating_type = boost::multiprecision::cpp_bin_float_oct;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    using table_type = std::unordered_map<K, T>;

    static toml::result<integer_type, toml::error_info>
    parse_int(const std::string& str, const toml::source_location src, const std::uint8_t base)
    {
        return toml::read_int<integer_type>(str, src, base);
    }
    static toml::result<floating_type, toml::error_info>
    parse_float(const std::string& str, const toml::source_location src, const bool is_hex)
    {
        return toml::read_float<floating_type>(str, src, is_hex);
    }
};
```



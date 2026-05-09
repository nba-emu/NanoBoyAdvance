+++
title = "find.hpp"
type  = "docs"
+++

# find.hpp

`toml::value`から値を検索し、同時に（必要な場合）型変換を行う関数です。

{{< hint info >}}

`toml::value` は格納する型を変更でき、`toml::find`はそれらに対応しているので、
厳密には全て `toml::basic_value<TC>` が使われています。ただしこれでは冗長なので、
関数の宣言と特に区別しなければならない場合を除いて、簡単のため説明文では `toml::value` と書きます。
説明文では、テンプレートパラメータ`TC`を変更して型が変更されていれば
`toml::value::integer_type` などの型は追従して変更されると解釈してください。

{{< /hint >}}

# `toml::find`

## 概要

`toml::find`には、取り出したい型をテンプレート引数で、検索したい値のキーを引数で与えます。

```cpp
template<typename T, typename TC, typename ... Keys>
T find(const basic_value<TC>& v, Keys ... keys);
```

サポートされている `T` の種類と変換の挙動に関しては、 `toml::get` と同様です。

`T` が指定されなかった場合、 `toml::value` が返されます。

キーは、 `toml::value::key_type` または `std::size_t` です。
複数個のキーが与えられた場合は、サブテーブルや配列に対して再帰的に検索が行われます。
`toml::value::key_type` が与えられた場合は `toml::table` として、 `std::size_t` が与えられた場合は `toml::array` として解釈されます。

### 再帰的な検索に関しての注意

TOMLには通常の bare key の他に、 `"` や `'` で囲まれた quoted key というものがあります。
quoted key を使うと、 `"foo.bar" = "baz"` というようなキーを書くことができ、この場合はサブテーブルは構築されず、キーは `foo.bar`となります。
このようなパターンに対応するため、toml11ではキーの中に`.`が含まれていても分割は行わず、そのままの文字列で検索を行います。

以下のTOMLファイルを考えます。

```toml
[foo]
[foo.bar]
baz = "hoge"

["foo.bar"]
baz = "fuga"
```

これに対応する`toml::find`の書き方は以下の通りです。

```cpp
const auto input = toml::parse("input.toml");
const auto baz1 = toml::find<std::string>(input, "foo", "bar", "baz"); // hoge
const auto baz2 = toml::find<std::string>(input, "foo.bar",    "baz"); // fuga
```

参考：[toml.io/ja/v1.0.0#キー](https://toml.io/ja/v1.0.0#%E3%82%AD%E3%83%BC)

## `toml::find(value, key)`

`value` を `toml::table` として `key` を検索したあと、 `toml::get` で変換を行います。

```cpp
template<typename T, typename TC>
/* toml::get<T>(const value&) と同等 */ find(
    const basic_value<TC>& v, const typename basic_value<TC>::key_type& ky);

template<typename T, typename TC>
/* toml::get<T>(value&) と同等 */ find(
    basic_value<TC>& v, const typename basic_value<TC>::key_type& ky);

template<typename T, typename TC>
/* toml::get<T>(value&&) と同等 */ find(
    basic_value<TC>&& v, const typename basic_value<TC>::key_type& ky);
```

`T`が指定されない場合は、変換を行わず`toml::value`を返します。

```cpp
template<typename TC>
basic_value<TC> const& find(
    basic_value<TC> const& v, const typename basic_value<TC>::key_type& ky);

template<typename TC>
basic_value<TC>& find(
    basic_value<TC>& v, const typename basic_value<TC>::key_type& ky);

template<typename TC>
basic_value<TC> find(
    basic_value<TC>&& v, const typename basic_value<TC>::key_type& ky);
```

### 例外

`toml::value` が `table` を保持していなかった場合、 `toml::type_error` が送出されます。

格納している `table` が指定された要素を持っていなかった場合、 `std::out_of_range` が送出されます。

指定された要素が `T` に変換できない場合 （ `toml::get` が変換に失敗する場合） 、
`toml::type_error` が送出されます。

## `toml::find(value, index)`

`value` を `toml::array` として `index` 番目にアクセスし、 `toml::get` で変換を行います。

```cpp
template<typename T, typename TC>
/* toml::get<T>(const value&) と同等 */ find(
    const basic_value<TC>& v, const std::size_t index);

template<typename T, typename TC>
/* toml::get<T>(value&) と同等 */ find(
    basic_value<TC>& v, const std::size_t index);

template<typename T, typename TC>
/* toml::get<T>(value&&) と同等 */ find(
    basic_value<TC>&& v, const std::size_t index);
```

`T`が指定されない場合は、変換を行わず`toml::value`を返します。

```cpp
template<typename TC>
basic_value<TC> const& find(basic_value<TC> const& v, const std::size_t ky);

template<typename TC>
basic_value<TC>& find(basic_value<TC>& v, const std::size_t ky);

template<typename TC>
basic_value<TC> find(basic_value<TC>&& v, const std::size_t ky);
```

### 例外

`toml::value` が `array` を保持していなかった場合、 `toml::type_error` が送出されます。

格納している `array` が指定された数の要素を持っていなかった場合、`std::out_of_range`が送出されます。

指定された要素が `T` に変換できない場合 （ `toml::get` が変換に失敗する場合） 、
`toml::type_error` が送出されます。

## `toml::find(value, keys...)`

```cpp
template<typename T, typename TC, typename K1, typename K2, typename ... Ks>
/* toml::get<T>(const value&) と同等 */ find(
    const basic_value<TC>& v, const K1& k1, const K2& k2, const Ks& ... ks);

template<typename T, typename TC, typename K1, typename K2, typename ... Ks>
/* toml::get<T>(value&) と同等 */ find(
    basic_value<TC>& v, const K1& k1, const K2& k2, const Ks& ... ks);

template<typename T, typename TC, typename K1, typename K2, typename ... Ks>
/* toml::get<T>(value&&) と同等 */ find(
    basic_value<TC>&& v, const K1& k1, const K2& k2, const Ks& ... ks);
```

再帰的に`toml::find`が呼び出されます。

失敗する条件とその際に送出される例外は `toml::find` と同じです。

# `toml::find_or(value, key, fallback)`

```cpp
template<typename T, typename TC, typename Key>
T find_or(const basic_value<TC>& v, const Key& key, T&& opt);
```

`find_or` は失敗した際のためのデフォルト値を受け取ることで、失敗時に例外を投げないようにします。

このデフォルト値は受け取る型`T`と同じ型でなければなりません。
よって、 `toml::find<T>` とは違って、 `find_or` では `T` を指定せずとも推論されます。

`find_or<T>`のように `T` を指定することもできますが、その場合は常に新規な値が返されます。
参照を取得したい場合は、 `T` を指定しないでください。

## `T`が`basic_value`である場合

```cpp
template<typename TC, typename K>
basic_value<TC>& find_or(basic_value<TC>& v, const K& key, basic_value<TC>& opt) noexcept

template<typename TC, typename K>
basic_value<TC> const& find_or(const basic_value<TC>& v, const K& key, const basic_value<TC>& opt) noexcept
```

対応する値を検索し、変換せずに返します。変換が必要ないため、参照を返すことができます。

値が見つからなかった場合、デフォルト値を返します。

## `T`が`toml::value::{some_type}`である場合

```cpp
template<typename T, typename TC, typename K>
T& find_or(basic_value<TC>& v, const K& key, T& opt) noexcept

template<typename T, typename TC, typename K>
T const& find_or(const basic_value<TC>& v, const K& key, const T& opt) noexcept
```

対応する値を検索し、変換せずに返します。変換が必要ないため、参照を返すことができます。

値が見つからなかった場合、あるいは異なる型が格納されていた場合、デフォルト値を返します。

## `T`が`const char*`である場合

```cpp
template<typename TC, typename K>
std::string find_or(const basic_value<TC>& v, const K& k, const char* opt)
```

対応する値を検索し、 `std::string` として返します。

失敗時に `const char*` から `std::string` を構築するため、参照を返すことはできません。

## `T`がその他の型である場合

```cpp
template<typename T, typename TC, typename K>
T find_or(const basic_value<TC>& v, const K& key, T opt)
```

対応する値を検索し、 `T` に変換して返します。

変換を行うため、参照を返すことはできません。

## 複数のキーが与えられた場合

```cpp
template<typename Value, typename K1, typename K2, typename K3, typename ... Ks>
auto find_or(Value&& v, const K1& k1, const K2& k2, K3&& k3, Ks&& ... keys) noexcept
 -> decltype(find_or(v, k2, std::forward<K3>(k3), std::forward<Ks>(keys)...))
```

キーの列の最後の要素がデフォルト値であると解釈して、再帰的に`find_or`を適用します。

`T` の推論結果が `toml::value` または `toml::value::some_type` になる場合、参照を返すことができます。

`T` を明示的に指定した場合、常に変換を行います。

# 関連項目

- [get.hpp]({{<ref "get.md">}})
- [value.hpp]({{<ref "value.md">}})

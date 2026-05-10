+++
title = "get.hpp"
type  = "docs"
+++

# get.hpp

`toml::value`から値を取り出し、同時に（必要な場合）型変換を行う関数です。

{{< hint info >}}

`toml::value` は格納する型を変更でき、`toml::get`はそれらに対応しているので、
厳密には全て `toml::basic_value<TC>` が使われています。ただしこれでは冗長なので、
関数の宣言と特に区別しなければならない場合を除いて、簡単のため説明文では `toml::value` と書きます。
説明文では、テンプレートパラメータ`TC`を変更して型が変更されていれば
`toml::value::integer_type` などの型は追従して変更されると解釈してください。

{{< /hint >}}

# `toml::get<T>`

## 概要

基本的に、`toml::get`は以下のような関数として振る舞います。
`T`は`toml::get<int>(v)`のようにして与えます。

```cpp
template<typename T, typename TC>
T get(const basic_value<TC>& v);
```

ただし、`T`がどのような型であるかによって、`toml::get`は異なる挙動をします。

`T`の型の種類は、

1. 変換が必要ない型
2. 変換する必要がある型

に分けられます。

細かい条件と、特別にサポートしている具体的な型については後述します。

### 変換が必要ない型

変換が必要ないのは、渡された `toml::value` が格納している型です。
例えば、 `toml::value::integer_type` は `std::int64_t` のエイリアスなので、
`toml::get<std::int64_t>(v)` は変換を必要としません。
この場合、 `toml:get` は `integer` の値を `toml::value` から取り出し、その参照を返します。

渡された`toml::value`が可変参照(`&`)である場合、返す値も可変参照(`&`)です。
不変参照(`const&`)の場合、返す値も不変参照(`const&`)となります。
可変参照を返した場合、その参照を通して`toml::value`に格納されている値に上書きできます。

### 変換が必要な型

上記の型以外については変換が必要です。
例えば、`toml::value::integer_type`は`std::int64_t`のエイリアスなので、`toml::get<std::size_t>(toml::value&)`は変換が必要です。
この場合、`toml:get`は`integer`の値を`toml::value`から取り出し、それをキャストして返します。

toml11は簡単なキャストだけでなく、
`toml::array`からや`std::tuple<int, double, std::string>`や`std::array<double, 4>`、
`toml::table`から`std::map<std::string, int>`などの複雑な型変換をサポートします。
具体的には、続くセクションを参照してください。

### 失敗した場合

期待した型変換を行えない場合があります。例えば、`table`を持っている`toml::value`に`toml::get<int>(v)`を適用した場合などです。

このような場合は、取り出そうとした型に最も似ている型への変換（今回は`int`なので、`as_integer`）が失敗したとして、`toml::type_error`が送出されます。

ファイルからパースした場合、以下のようなエラーメッセージが出力されます。

```
terminate called after throwing an instance of 'toml::type_error'
  what():  toml::value::as_integer(): bad_cast to integer
 --> input.toml
   |
 6 | [fruit]
   | ^^^^^^^-- the actual type is table
```

## `T`が`toml::value`と同一のとき

```cpp
template< T, typename TC>
T& get(basic_value<TC>& v);

template<typename T, typename TC>
T const& get(const basic_value<TC>& v);

template<typename T, typename TC>
T get(basic_value<TC>&& v);
```

条件：
- `std::is_same<T, basic_value<TC>>` を満たす

`toml::value`から`toml::value`なので、変換は行われず、そのままの値が返されます。
他の関数の実装を一般化するためだけに存在しています。

失敗しません。

## `T`が`toml::value::{some_type}`のいずれかのとき

```cpp
template<typename T, typename TC>
T& get(basic_value<TC>& v);

template<typename T, typename TC>
T const& get(const basic_value<TC>& v);

template<typename T, typename TC>
T get(basic_value<TC>&& v);
```

条件：
- `T`が`toml::value`が格納できる型（`toml::value::boolean_type`など）のどれかと同一であること

`toml::value` が格納している型と同じ型、例えば `toml::value::integer_type` が
`toml::get<T>` の `T` として指定されたとき、型変換の必要がないため参照を返すことが可能です。

異なる型が格納されていた場合、`toml::type_error` が送出されます。

## `T`が異なる`TypeConfig`を持つ`basic_value<OtherTC>`のとき

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `T`が`toml::basic_value<TC>`ではない
- `T`が`toml::basic_value<OtherTC>`である

異なる型を格納し得る`basic_value`が指定された場合、変換が行われます。

型変換が発生するので、返す値は新規な値であり、参照ではありません。

失敗しません（メモリ枯渇などの場合を除く）。

## `T`が整数型の場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `std::is_integral<T>` を満たす
- `std::is_same<T, bool>` ではない
- `toml::value::integer_type` ではない

`toml::value` を `integer_type` を保持しているとしてその値を取得し、それを `T` に変換して返します。

`toml::value::integer_type` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

## `T`が浮動小数点数型の場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `std::is_floating_point<T>` を満たす
- `toml::value::floating_type` ではない

`toml::value`を`floating_type`を保持しているとしてその値を取得し、それを`T`に変換して返します。

`toml::value::floating_type` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

## `T`が`std::string_view`の場合

C++17以降でのみ使用可能です。

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `std::is_same<std::string_view, T>` を満たす

`toml::value`を`string_type`を保持しているとしてその値を取得し、それから`std::string_view`を構築して返します。

`toml::value::string_type` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

## `T`が`std::chrono::duration`の場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `T`が`std::chrono::duration<Rep, Period>`である

`toml::value`を`local_time`を保持しているとしてその値を取得し、それを`std::chrono::duration`に変換して返します。

`toml::value::local_time` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

## `T`が`std::chrono::system_clock::time_point`の場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `std::is_same<T, std::chrono::system_clock::time_point>`を満たす

`toml::value` を `local_date`, `local_datetime`, `offset_datetime`のどれかを保持しているとしてその値を取得し、それを`std::chrono::system_clock::time_point`に変換して返します。

`local_date`, `local_datetime`, `offset_datetime` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

## `T`が`array-like`である場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `T`は`T::iterator`を持つ
- `T`は`T::value_type`を持つ
- `T`は`T::push_back(x)`を持つ
- `T`は`toml::value::array_type`ではない
- `T`は`std::string`ではない
- `T`は`std::string_view`ではない
- `T`は`map-like`ではない
- `T`は`from_toml()`メンバ関数を持たない
- `toml::from<T>`が定義されていない
- `toml::basic_value<TC>`からのコンストラクタが定義されていない

`std::vector<int>`や`std::deque<std::string>`などが該当します。

`toml::value`を`array`を保持しているとしてその値を取得し、それを指定されたコンテナに変換して返します。

`toml::value::array_type` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

## `T`が`std::array`である場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `T`は`std::array<U, N>`である

`toml::value`を`array`を保持しているとしてその値を取得し、それを指定されたコンテナに変換して返します。

`toml::value::array_type` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

`toml::value` が持つ `array` が十分な数の要素を持っていなかった場合、`std::out_of_range`が送出されます。

## `T`が`std::forward_list`である場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `T`は`std::forward_list<U>`である

`toml::value`を`array`を保持しているとしてその値を取得し、それを`std::forward_list`に変換して返します。

`toml::value::array_type` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

`forward_list`は`push_back`を持たないので、別に実装されています。

## `T`が`std::pair`である場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `T`は`std::pair<T1, T2>`である

`toml::value`を`array`を保持しているとしてその値を取得し、それを`std::pair<T1, T2>`に変換して返します。

`first` と `second` はそれぞれ再帰的に変換されます。

`toml::value::array_type` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

`toml::value` が持つ `array` の要素数がちょうど2個でなかった場合、`std::out_of_range`が送出されます。

## `T`が`std::tuple`である場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `T`は`std::tuple<T1, T2, ... TN>`である

`toml::value`を`array`を保持しているとしてその値を取得し、それを`std::tuple<T1, T2, ...TN>`に変換して返します。

各要素はそれぞれ再帰的に変換されます。

`toml::value::array_type` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

`toml::value` が持つ `array` の要素数がちょうど `std::tuple_size<T>::value` 個でなかった場合、`std::out_of_range`が送出されます。

## `T`が`map-like`である場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `T`は`T::iterator`を持つ
- `T`は`T::key_type`を持つ
- `T`は`T::value_type`を持つ
- `T`は`T::mapped_type`を持つ
- `T`は`toml::value::table_type`ではない
- `T`は`from_toml()`メンバ関数を持たない
- `toml::from<T>`が定義されていない
- `toml::basic_value<TC>`からのコンストラクタが定義されていない

`std::map<std::string, int>`や`std::unordered_map<std::string, float>`などが該当します。

`toml::value`を`table`を保持しているとしてその値を取得し、それを `T` に変換して返します。

各要素はそれぞれ再帰的に変換されます。

`basic_value::table_type` 以外の型が格納されていた場合、 `toml::type_error` が送出されます。

## `T`が`toml::from<T>`の特殊化を持つユーザー定義型である場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `toml::from<T>`が定義されている

`toml::from` の `T` に対する特殊化が定義されていた場合、それを使用した型変換が行われます。

個別にサポートされる型（ `std::array`, `std::pair`, `std::tuple` ）と衝突しないようにしてください。

## `T`が`T::from_toml`メンバ関数を持つユーザー定義型である場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `toml::from<T>`が定義されていない
- `T`は`from_toml()`メンバ関数を持つ

`T` に `from_toml(toml::basic_value<TC>)` メンバ関数が定義されていた場合、それを使用した型変換が行われます。

`toml::from<T>` が定義されていると、そちらが優先されます。

## `T`が`toml::basic_value<TC>`を取るコンストラクタを持つユーザー定義型である場合

```cpp
template<typename T, typename TC>
T get(basic_value<TC>& v);
```

条件：
- `toml::from<T>`が定義されていない
- `T`は`from_toml()`メンバ関数を持たない
- `T`は`toml::basic_value<TC>`を取るコンストラクタを持つ

`T` に `toml::basic_value<TC>` を取るコンストラクタが定義されていた場合、それを使用した型変換が行われます。

`toml::from<T>` または `T::from_toml` が定義されていると、そちらが優先されます。

# `toml::get_or<T>`

`get_or` は失敗した際のためのデフォルト値を受け取ることで、失敗時に例外を投げないようにします。

このデフォルト値は受け取る型`T`と同じ型でなければなりません。
よって、 `toml::get<T>` とは違って、 `get_or` では `T` を指定せずとも推論されます。

## `T`が`basic_value<TC>`である場合

```cpp
template<typename TC>
basic_value<TC> const& get_or(const basic_value<TC>& v, const basic_value<TC>& opt)

template<typename TC>
basic_value<TC> & get_or(basic_value<TC>& v, basic_value<TC>& opt)

template<typename TC>
basic_value<TC> get_or(basic_value<TC>&& v, basic_value<TC>&& opt)
```

変換先は同一の`toml::value`なので、失敗しません。

他の関数の実装を一般化するためだけに存在しています。

## `T`が`basic_value<TC>::{some_type}`である場合

```cpp
template<typename T, typename TC>
T const& get_or(const basic_value<TC>& v, const T& opt) noexcept

template<typename T, typename TC>
T & get_or(basic_value<TC>& v, T& opt) noexcept

template<typename T, typename TC>     
T get_or(basic_value<TC>&& v, T&& opt) noexcept
```

`toml::get<T>`と同様の変換を行います。失敗した場合は第二引数が返されます。

## `T`が`const char*`である場合

```cpp
template<typename TC>
typename basic_value<TC>::string_type
get_or(const basic_value<TC>& v,
       const typename basic_value<TC>::string_type::value_type* opt);
```

`const char*` が渡された場合、変換先は `std::string` として解釈されます。

## `T`がその他の場合

```cpp
template<typename TC>
typename std::remove_cv<typename std::remove_reference<T>::type>::type
get_or(const basic_value<TC>& v, T&& opt);
```

`toml::get<T>`と同様の変換を行います。失敗した場合は第二引数が返されます。

# 関連項目

- [find.hpp]({{<ref "find.md">}})
- [from.hpp]({{<ref "from.md">}})
- [value.hpp]({{<ref "value.md">}})


+++
title = "getting values"
type  = "docs"
weight = 20
+++

# 値を取り出す

ここでは、 `toml::value` が格納している値にアクセスする方法を説明します。

## メンバ関数を使って値にアクセスする

### `is_something` と `as_something`

`toml::value` は `is_boolean()` や `is_integer()` などのメンバ関数を持っており、
これらを使うと持っている型を調べることができます。

また、 `as_boolean()`, `as_integer()` などのメンバ関数も持っており、
これらを使ってその型にアクセスすることができます。

完全なリストは [`toml::value` のリファレンス]({{<ref "docs/reference/value#is_xxx">}}) を参照してください。

```cpp
toml::value v = /* ... */;
if(v.is_integer())
{
    std::cout << v.as_integer() << std::endl;
}
```

指定された値と異なる型が格納されていた場合、 [`toml::type_error`]({{<ref "docs/reference/value#tomltype_error">}}) が送出されます。

その `what()` は以下のようなメッセージを含みます。

```
[error] toml::value::as_string(): bad_cast to string
 --> input.toml
   |
 1 | a = 123_456
   |     ^^^^^^^-- the actual type is integer
```

### `toml::value_t`

型情報は [`enum class toml::value_t`]({{<ref "docs/reference/value_t">}}) で識別できます。

[`type()`]({{<ref "docs/reference/value#type">}}) メンバ関数は、現時点で格納している値の型情報を返します。

```cpp
toml::value v = /* ... */;
switch(v.type())
{
    case toml:value_t::empty          : { /*...*/ break; }
    case toml:value_t::boolean        : { /*...*/ break; }
    case toml:value_t::integer        : { /*...*/ break; }
    case toml:value_t::floating       : { /*...*/ break; }
    case toml:value_t::string         : { /*...*/ break; }
    case toml:value_t::offset_datetime: { /*...*/ break; }
    case toml:value_t::local_datetime : { /*...*/ break; }
    case toml:value_t::local_date     : { /*...*/ break; }
    case toml:value_t::local_time     : { /*...*/ break; }
    case toml:value_t::array          : { /*...*/ break; }
    case toml:value_t::table          : { /*...*/ break; }
    default: {break;}
}
```

[`is(toml::value_t)`]({{<ref "docs/reference/value#istomlvalue_t">}}) メンバ関数は、渡された `value_t` と同じ型の値を格納している場合 `true` を、
それ以外の場合 `false` を返します。

```cpp
toml::value v = /* ... */;
if(v.is(toml::value_t::integer))
{
    std::cout << v.as_integer() << std::endl;
}
```

### `at`, `[]`, `contains`, `size`, `push_back`, `emplace_back`

標準ライブラリコンテナが持つメンバ関数の一部は、 `toml::value` も提供しています。

これらは、内部で `toml::value` を対応する型に変換し、そのメンバ関数を呼び出します。

#### `at(std::size_t i)`, `operator[](std::size_t i)`

`as_array().at(i)`, `as_array()[i]` と同等です。

`toml::value` はデフォルトで `std::vector<toml::value>` を `array_type` に使うので、
エラーが発生した際には `at` は `std::out_of_range` を送出し、`operator[]` は未定義動作となります。

```cpp
toml::value v(toml::array{1,2,3});
std::cout << v.at(1);
```

格納している型が `array_type` ではなかった場合、 `type_error` を送出します。

#### `at(std::string key)`, `operator[](std::string key)`

`as_table().at(key)`, `as_table()[key]` と同等です。

`toml::value` はデフォルトで `std::unordered_map<std::string, toml::value>` を `table_type` に使うので、
対応する値が存在しない場合は `at` は `std::out_of_range` を送出し、 `operator[]` は新しく `toml::value` を構築してそれへの参照を返します。
そのため、`operator[]` に `const` 版はありません。

```cpp
toml::value v(toml::table{});
v["a"] = 42;
```

格納している型が `table_type` ではなかった場合、 `type_error` を送出します。

#### `size()`

長さを返します。

`array_type` または `table_type` の場合は要素数、 `string_type` の場合は文字数を返します。

格納している型がどれでもなかった場合、 `type_error` を送出します。

#### `push_back()`, `emplace_back()`

`as_array().push_back()`, `as_array().emplace_back()` と同一です。

格納している型が `array_type` ではなかった場合、 `type_error` を送出します。

## コメントにアクセスする

toml11では、デフォルトでコメントがパースされ、対応する値に行ごとに保存されます。

対応する値は、連続するコメント行の直後に来る値か、もしくはそのコメントと同じ行に描かれている値です。

直前または直後に値がなければ、そのコメントはどこにも紐づけられず、無視されます。

```toml
# input.toml

# これはaに関するコメントです。
a = 42
b = 3.14 # これはbに関するコメントです。

# このコメントには対応する値がないため無視されます。

# これは1番目のcに関するコメントです。
# これは2番目のcに関するコメントです。
c = "foo" # これは最後のcに関するコメントです。
```

値に対応するコメントには、`toml::value` の `comments()` メンバ関数を使ってアクセスします。

`comments()` は `std::vector<std::string>` と同じメンバ関数を持つコンテナを返します。

```cpp
const auto v = toml::parse("input.toml");
const auto& a = v.at("a");
const auto& b = v.at("b");
const auto& c = v.at("c");

assert(a.comments().size() == 1);
assert(a.comments().at(0) == "# これはaに関するコメントです。");

assert(b.comments().size() == 1);
assert(b.comments().at(0) == "# これはbに関するコメントです。");

assert(c.comments().size() == 3);
assert(c.comments().at(0) == "# これは1番目のcに関するコメントです。");
assert(c.comments().at(1) == "# これは2番目のcに関するコメントです。");
assert(c.comments().at(2) == "# これは最後のcに関するコメントです。");
```

ファイル全体に対応するルートテーブルに関するコメントは、ファイルの先頭に書きます。

```toml
# ルートテーブルに関するコメントです。
# これもルートテーブルに関するコメントです。

# これはaに関するコメントです。
a = 42
```

ただし、もしファイルの先頭のコメントの直後に値が来た場合、
そのコメントはその値に関するコメントと解釈され、ルートテーブルのコメントはなくなります。

```toml
# これはaに関するコメントです。
# これもaに関するコメントです。
a = 42
```

## インラインテーブル・ドットキーの取り扱い

インラインテーブルは単にテーブルで、C++コード上で他のテーブルと異なる点はありません。

```toml
a = {b = 42, c = "foo"}
```


ドットキーも単にテーブルで、C++コード上で他のテーブルと異なる点はありません。

```toml
a.b = 42
a.c = "foo"
```

これらは以下のファイルと全く同じ構造を持ちます。

```toml
[a]
b = 42
c = "foo"
```

なので、どの記法でも以下の全く同一のコードで処理できます。

```cpp
const auto input = toml::parse("input.toml");

assert(input.at("a").at("b").as_integer() == 42);
assert(input.at("a").at("c").as_string()  == "foo");
```

ただし、フォーマット情報によって区別することは可能です。

```cpp
const auto input = toml::parse("input.toml");
switch(input.at("a").as_table_fmt().fmt)
{
    case toml::table_format::oneline: 
    {
        std::cout << "inline table" << std::endl;
        break;
    }
    case toml::table_format::multiline:
    {
        std::cout << "normal table" << std::endl;
        break;
    }
    case toml::table_format::dotted:
    {
        std::cout << "dotted keys" << std::endl;
        break;
    }
}
```

このフォーマット情報は後述するシリアライズの際も考慮されます。

## 日付情報の取り扱い

[`local_date`]({{<ref "docs/reference/datetime#local_date">}}),
[`local_time`]({{<ref "docs/reference/datetime#local_time">}}),
[`local_datetime`]({{<ref "docs/reference/datetime#local_datetime">}}), そして
[`offset_datetime`]({{<ref "docs/reference/datetime#offset_datetime">}}) は、
toml11では対応するメンバ変数を持つ専用の構造体にパースされます。

使用する際は、直接値を取り出す他にも、後述する `toml::get` や `toml::find` を使用して、
`std::chrono::system_clock::time_point` や `std::tm` 等の型に変換することができます。

## `toml::get<T>`を使って変換する

`toml::get<T>` は、 `toml::value` の持つ値を変換して取り出す関数です。
`T` に変換先に指定したい型を指定します。

```cpp
const toml::value v = /*...*/;
std::cout << toml::get<int>(v) << std::endl;
```

後述する `toml::find<T>` も、型変換の部分は同一の機能を持ちます。

格納されている型のそれぞれについて、
変換ができない型が指定された場合、 `toml::type_error` が送出されます。

### 単純な変換

#### boolean_type

`boolean_type` から変換が可能なのは、 `bool` のみです。

#### integer_type

`bool` 以外で `std::is_integral<T>` が `true` になる型は、 `integer_type` から変換できます。

```cpp
toml::value v(42);
const auto u32 = toml::get<std::uint32_t>(v);
const auto i16 = toml::get<short>(v);
```

#### floating_type

`std::is_floating_point<T>` が `true` になる型は、`floating_type` から変換できます。

```cpp
toml::value v(3.14);
const auto f64 = toml::get<double>(v);
const auto f32 = toml::get<float >(v);
```

#### string_type

`string_type` からは `std::string` へ変換できます。
また、C++17以降では、`std::string_view` へも変換できます。

```cpp
toml::value v("foo");
const auto s = toml::get<std::string>(v);

// C++17以降
const auto sv = toml::get<std::string_view>(v);
```

#### datetime variants

[`local_date`]({{<ref "docs/reference/datetime#local_date">}}),
[`local_datetime`]({{<ref "docs/reference/datetime#local_datetime">}}),
[`offset_datetime`]({{<ref "docs/reference/datetime#offset_datetime">}}) は
ある日付と時刻を指しているため、
`std::chrono::system_clock::time_point` への変換が可能です。

ただし、[`local_time`]({{<ref "docs/reference/datetime#local_time">}}) は
日付の情報がないため、0時0分からの経過時刻として `std::chrono::duration` への
変換をサポートします。

また、 `local_date` と `local_datetime` は実行中のマシンのタイムゾーンを取得して変換を行います。

```toml
date = 2024-01-23
time = 12:30:00
l_dt = 2024-01-23T12:30:00
o_dt = 2024-01-23T12:30:00+09:00
```

```cpp
const auto input = toml::parse("input.toml");

const auto date = toml::get<std::chrono::system_clock::time_point>(input.at("date"));
const auto l_dt = toml::get<std::chrono::system_clock::time_point>(input.at("l_dt"));
const auto o_dt = toml::get<std::chrono::system_clock::time_point>(input.at("o_dt"));

const auto time = toml::get<std::chrono::minutes>(input.at("time")); // 12 * 60 + 30 min
```

### 参照を取得できる条件

`toml::get<T>` は、 `T` が `toml::value` が格納する型そのものだった場合、参照を返すことができます。

逆に、変換が必要な場合（ `std::int64_t` で格納されている整数を `std::uint32_t` で取り出そうとした場合）は、
変換後の型への参照を返すことは不可能です。

変換が必要ない型の場合、返された参照を経由して値を書き換えることも可能です。

```cpp
toml::value v(42);

toml::get<toml::value::integer_type>(v) = 6 * 9;

assert(v.as_integer() == 54);
```

### 配列をSTLコンテナに

配列の要素型が全て同じ場合、要素型が `T` に変換可能であれば、 `std::vector<T>` に変換可能です。

```toml
a = [1, 2, 3, 4, 5]
```

```cpp
const auto a = toml::get<std::vector<int>>(input.at("a"));
```

他のSTLコンテナにも変換可能です。

```cpp
const auto a1 = toml::get<std::deque<int>>(input.at("a"));
const auto a2 = toml::get<std::list <int>>(input.at("a"));
const auto a3 = toml::get<std::array<int, 5>>(input.at("a"));
```

`std::array` に変換する場合、要素数が一致している必要があります。
もし要素数が一致しなかった場合、 `std::out_of_range` が送出されます。

STL以外のコンテナであっても、デフォルトコンストラクタと `push_back` を持っている場合、
`toml::get` で変換が可能です。

```cpp
const auto a = toml::get<boost::container::small_vector<int, 8>>(input.at("a"));
```

### 配列を `std::pair`, `std::tuple` に

配列の要素型が異なる場合、 `std::pair` や `std::tuple` に変換が可能です。

```toml
a = [true, 3.14]
b = [42, 2.718, "foo"]
```

```cpp
const auto a = toml::get<std::pair<bool, double>>(input.at("a"));
const auto b = toml::get<std::tuple<int, double, std::string>>(input.at("b"));
```

`std::array` の場合と同様に、配列の長さは `std::pair`, `std::tuple` の要素数と一致している必要があります。
もし要素数が一致しなかった場合、 `std::out_of_range` が送出されます。

また、各要素は対応する要素に変換できる必要があります。
変換できない場合、 `toml::type_error` が送出されます。

### ネストされた配列の変換

ネストされた配列は、ネストされたコンテナに変換可能です。

```toml
a = [ [1, 2, 3], [4, 5, 6] ]
```

```cpp
const auto a = toml::get<std::vector<std::vector<int>>>(input.at("a"));
```

型が異なる場合には、 `std::pair/tuple` が便利です。

```toml
a = [ [1, 2, 3], ["foo", "bar"] ]
```

```cpp
const auto a = toml::get<
    std::pair<std::vector<int>, std::vector<std::string>>
    >(input.at("a"));
```

### テーブルを `std::map` に変換

テーブルに含まれる値の型が全て同じであれば、 `std::map` や `std::unordered_map` に変換が可能です。

```toml
t = {a = 1, b = 2}
```

```cpp
const auto t = toml::get<std::map<std::string, int>>(input.at("t"));
```

STL以外のコンテナであっても、デフォルトコンストラクタと `emplace(key, mapped)` を持っている場合、
`toml::get` で変換が可能です。

```cpp
const auto t = toml::get<boost::container::flat_map<std::string, int>>(input.at("t"));
```

要素型の変換に失敗した場合は `toml::type_error` が送出されます。

## `toml::get_or`を使って失敗時の値を指定する

`toml::get` は変換に失敗した際に `toml::type_error` 例外を送出します。

`toml::get_or` を使用することで、変換に失敗した際に例外ではなくデフォルト値を返せるようになります。

`toml::get<T>` とは異なり、 `get_or` は引数から変換先の型を推論するため、 `<T>` の指定は不要です。

```cpp
const auto a = toml::get_or(input.at("a"), 42);
```

変換可能な型は `toml::get` と同様です。

`toml::value::xxx_type`を指定した場合は、参照を取り出すことも可能ですが、その場合は引数も参照である必要があります。

```cpp
toml::value::integer_type a_default = 42;
auto a& = toml::get_or(input.at("a"), a_default);
```

## `toml::find<T>`を使って検索と変換を同時に行う

`toml::find<T>` は、テーブルを持つ `toml::value` から値を検索し、同時に
`toml::get` と同じ型変換を行って取り出す関数です。

```cpp
const auto a = toml::find<int>(input, "a");
// const auto a = toml::get<int>(input.at("a")); と同じ
```

`toml::find<T>` は配列にも使用できます。

```cpp
const auto a  = input.at("a");
const auto a2 = toml::find<int>(a, 2);
// const auto a2 = toml::get<int>(input.at("a").at(2)); と同じ
```

型変換の際にエラーが起きた場合、 `toml::get` と同じ `toml::type_error` を
送出します。
キーが見つからなかった場合またはインデックスが存在しない場合は、
`std::out_of_range` を送出します。

型を指定しなかった場合、型変換を行わず `toml::value` を取り出します。

```cpp
const auto a = toml::find(input, "a");
// const auto a = input.at("a"); と同じ
```

`toml::find<T>` は再帰的に値にアクセスることもできます。

```toml
a = {b = {c = 42}}
```

```cpp
const auto a_b_c = toml::find<int>(input, "a", "b", "c");
// const auto a = toml::get<int>(input.at("a").at("b").at("c")); と同じ
```

このとき、キーとインデックスを混ぜることができます。


```toml
a = [ {b = 1}, {b = 2}, {b = 3} ]
```

```cpp
const auto a_2_b = toml::find<int>(input, "a", 2, "b");
// const auto a = toml::get<int>(input.at("a").at(2).at("c")); と同じ
```

{{<hint info>}}

TOMLはquoted keyという機能を持っています。

これは、 `""` や `''` を使うことで通常許可されない文字をキーに使えるというもので、
この中では `.` はテーブルを導入**しません**。

```toml
"127.0.0.1" = "value"
site."google.com" = true
```

このTOMLファイルは以下のようにして読みます。

```cpp
const auto input = toml::parse("input.toml");

assert(input.at("127.0.0.1").as_string() == "value");
assert(input.at("site").at("google.com").as_boolean());
```

このような場合にも違和感なく対応するため、toml11ではキーに `.` が含まれていても
自動で分割はしません。

テーブルの階層構造を陽に指定することが、適切な入力ファイルの構造化に資すると考えているからです。

参考: [toml.io キー](https://toml.io/ja/v1.0.0#%E3%82%AD%E3%83%BC)

{{</hint>}}

## `toml::find_or`を使って失敗時の値を指定する

`toml::find_or` は、 `toml::get_or` と同様に、失敗時のデフォルト値を渡します。

```cpp
const auto a = toml::find_or(input, "a", 42);
```

型変換の失敗だけでなく、キーが見つからなかった場合もデフォルト値を返します。

## `toml::find_or_default`を使って失敗時の値を指定する

`toml::find_or_default` は、 `toml::find_or` と同様に、失敗時にデフォルトコンストラクタの結果を返します。デフォルトコンストラクタは失敗時のみに呼ばれるため、特にそれが高価な時に有効です。

```cpp
const auto a = toml::find_or(input, "a", expensive());  // 関数呼出前にコンストラクタ呼出
const auto a = toml::find_or_default<expensive>(input, "a");  // 失敗時にのみコンストラクタ呼出
```

型変換の失敗だけでなく、キーが見つからなかった場合もデフォルトコンストラクタの結果を返します。

## `toml::find<std::optional<T>>`

C++17以降の場合、`std::optional`を`toml::find`に指定することができます。

`find`と同様に、再帰的なアクセスも可能です。

```cpp
const auto input = toml::parse_str(R"(
integer = 1

[table]
key = 2

[[array-of-tables]]
key = 3
)");

const auto a = toml::find<std::optional<int>>(input, "integer");
const auto b = toml::find<std::optional<int>>(input, "table", "key");
const auto c = toml::find<std::optional<int>>(input, "array-of-tables", 0, "key");
```

キーが存在しなかった場合、例外は投げられず、`std::nullopt`が返却されます。

ただし、型変換が失敗した場合や、テーブルではない値にキーでアクセスしようとした場合、配列でない値にインデックスでアクセス仕様とした場合は、`toml::type_error`が送出されます。

## ユーザー定義型との変換を定義する

`toml::get` や `toml::find` では、以下のどれかの方法を使うことで
ユーザー定義型を使用することができます。

### `toml::from` の定義

toml11には `toml::from` という型があり、以下のように特殊化することでユーザー定義型からの
変換をサポートできます。

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib

namespace toml
{
template<>
struct from<extlib::foo>
{
    static extlib::foo from_toml(const toml::value& v)
    {
        return extlib::foo{
            toml::find<int>(v, "a"),
            toml::find<std::string>(v, "b")
        };
    }
};
} // toml
```

後述する型設定を変更した `toml::value` もサポートする場合、以下のようにしてください。

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib

namespace toml
{
template<>
struct from<extlib::foo>
{
    template<typename TC>
    static extlib::foo from_toml(const toml::basic_value<TC>& v)
    {
        return extlib::foo{
            toml::find<int>(v, "a"),
            toml::find<std::string>(v, "b")
        };
    }
};
} // toml
```

この定義は、 `TOML11_DEFINE_CONVERSION_NON_INTRUSIVE` によって自動的に定義できます。

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib

TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(extlib::foo, a, b)
```

あるいは、リフレクションライブラリを使用することもできます。
`example` の `boost-ext/reflect` を使用したサンプルも参照してください。

### `from_toml` メンバ関数の定義

`from_toml` メンバ関数を定義することによっても変換を定義することができます。

これを使用する場合、デフォルトコンストラクタが必要です。

```cpp
struct bar
{
    int a;
    std::string b;

    void from_toml(const toml::value& v)
    {
        this->a = toml::find<int>(v, "a");
        this->b = toml::find<std::string>(v, "b");
        return ;
    }
};
```

両方が定義されていた場合、 `toml::from` が優先されます。

### `toml::value` を受け取るコンストラクタ

`toml::value` を受け取るコンストラクタがあれば、 `toml::get` による変換ができます。

```cpp
struct baz
{
    explicit baz(const toml::value& v)
        : a(toml::find<int>(v, "a")), b(toml::find<std::string>(v, "b"))
    {}
    int a;
    std::string b;
};
```

両方が定義されていた場合、`toml::from` と `from_toml` が優先されます。

## `toml::visit`で関数を適用する

`toml::value` が格納する型すべてに適用できる関数オブジェクトがあれば、
`toml::visit` によって型変換を経ずに直接その関数を呼ぶことができます。

```cpp
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

toml::value v(3.14);
std::cout << toml::visit(type_name_of{}, v) << std::endl; // floating
```

## `toml::value` を構築する

`toml::value` はパーサの内部だけでなく、ユーザーコードで構築することも可能です。

`toml::value` が格納する型と同じか変換可能な型を渡しての構築が可能です。

```cpp
toml::value v1(true);
toml::value v2(42);
toml::value v3(3.14);
```

配列の場合、 `toml::array` を使うか、

```cpp
toml::value v(toml::array{1, 2, 3});
```

配列の場合、 `std::vector` などのコンテナを直接渡すことが可能です。

```cpp
const std::vector<toml::value> a{1,2,3};
toml::value v(a);
```

このコンテナには、 `toml::get` で変換可能なコンテナが使用できます。

テーブルの場合も同様に、 `toml::table` を使うか、

```cpp
toml::value v(toml::table{{"foo", 1}, {"bar", 2}, {"baz", 3}});
```

`std::map` などのコンテナを直接渡します。

```cpp
const std::map<std::string, toml::value> t{
        {"foo", 1}, {"bar", 2}, {"baz", 3}
    }
toml::value v(t);
```

コンストラクタには、 `format_info` と コメントを渡すことができます。

コメントの型は `std::vector<std::string>` です。
各要素が一行分に相当します。

```cpp
toml::integer_format_info fmt;
fmt.fmt = toml::integer_format::hex;
fmt.spacer = 4;

toml::value v1(0xDEADBEEF, fmt);
toml::value v2(0xC0FFEE, fmt, {"hex value!"});
```

## `toml::value` に変換する

ユーザー定義型から `toml::value` を構築する際に、 `toml::into` や `into_toml` を
定義することで、その挙動をカスタマイズできます。

特に、別のライブラリの型などを変換する際に `toml::into` が便利です。

### `toml::into`を定義する

`toml::into` を特殊化することで `toml::value` への変換が可能になります。

`toml::value` への変換が用意されていない外部ライブラリの型などに対して有効です。

`toml::value` が変換時に `type_config` を渡すため、`basic_value` の `template` 引数を受け取る必要があります。

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib

template<>
struct into<extlib::foo>
{
    template<typename TC>
    static toml::basic_value<TC> into_toml(const extlib::foo& f)
    {
        return toml::basic_value<TC>(typename toml::basic_value<TC>::table_type{{"a", f.a}, {"b", f.b}});
    }
};
```

### `into_toml` メンバ関数を定義する

`from_toml` と同様、メンバ関数によっても変換を定義することができます。

`toml::into` が定義されていた場合、そちらが優先されます。

```cpp
struct bar
{
    int a;
    std::string b;

    toml::value into_toml() const
    {
        return toml::value(toml::table{{"a", this->a}, {"b", this->b}});
    }
};
```

# 値がアクセス済みかどうかチェックする

{{% hint warning %}}

この機能はデフォルトでは有効化されず、使用する際には`TOML11_ENABLE_ACCESS_CHECK`を定義する必要があります。
また、この機能はパースした値に対して追加の処理を行うため、実行時パフォーマンスが低下する可能性があります。

{{% /hint %}}

`TOML11_ENABLE_ACCESS_CHECK`マクロを定義してコンパイルすると、`toml::value`に`bool accessed() const`メソッドが追加され、パース後にその値にアクセスしたかどうかが確認できるようになります。

```console
$ g++ -std=c++17 -O2 -DTOML11_ENABLE_ACCESS_CHECK -I/path/to/toml11/include main.cpp
```

```console
$ cmake -B ./build -DTOML11_ENABLE_ACCESS_CHECK=ON
```

```cmake
CPMAddPackage(
    NAME toml11
    GITHUB_REPOSITORY "ToruNiina/toml11"
    VERSION 4.4.0
    OPTIONS "CMAKE_CXX_STANDARD 17" "TOML11_PRECOMPILE ON" "TOML11_ENABLE_ACCESS_CHECK ON"
    )
```

この機能によって、テーブル内に定義されているものの使用されなかった値についての警告を表示することが可能になります。

```cpp
#include <toml.hpp>

namespace yours
{

Config read_config(const toml::value& v)
{
    const auto cfg = read_your_config(input);

    for(const auto& [k, v] : input.as_table())
    {
        if( ! v.accessed())
        {
            std::cerr << toml::format_error("value defined but not used",
                v.source_location(), "not used");
        }
    }
    return cfg;
}
} // yours
```

この機能は、必要な場合のみ定義されるような値を、名称を間違えて定義してしまった際に役に立つでしょう。
例えば、

```toml
# 正しくは reactions 
# reactions = [ ":+1:", "star" ]

# 名前が違うので読み込めない
reaction = [ ":+1:", "star" ]
```

このファイルを上記のコードで読んだ場合、`read_your_config`は`reactions`を探し、定義されていなかったので空の配列として処理するでしょう。
その場合、`reaction`は`accessed()`が`true`にならないため、エラーとして検出できます。


+++
title = "serializing values"
type  = "docs"
weight = 40
+++

# TOMLファイルを出力する

`toml::format` を使うと、 `toml::value` を文字列にすることができます。

```cpp
#include <toml.hpp>
#include <cassert>

int main()
{
    const toml::value v(toml::table{
        {"a", 42},
        {"b", "foo"},
    });
    const std::string s = toml::format(v);

    const toml::value u = toml::parse_str(s);

    assert(u.at("a").as_integer() == 42);
    assert(u.at("b").as_string()  == "foo");

    return 0;
}
```

`table_type` を格納している `toml::value` が渡されると、それがファイルのルートテーブルとして解釈されます。

もし `table_type` 以外を格納している `toml::value` が渡されると、その値だけがフォーマットされます。

一部のフォーマット指定では、キーが渡されていないとフォーマットできないことがあります。
例えば、 `toml::array_format::array_of_tables` は `[[array.of.tables]]` の形でフォーマットするので、
キーへのアクセスを要求します。

キーを要求するフォーマット指定の値がキーなしで渡された場合、 `toml::serialization_error` が送出されます。

他にも、フォーマット指定と矛盾する値が含まれる場合には、 `toml::serialization_error` が送出されます。
例えば、 `integer_format::hex` が指定された整数が負の値を持っている場合や、
`string_format::literal` が指定された文字列が改行を含んでいる場合などです。

フォーマットの指定方法は後述します。

## キーを渡して出力する

`toml::format` には `std::string` としてキーを渡すことが可能です。

その場合、ルートテーブルの下にそのキーがあり、渡した値はそのキーに対応すると解釈されます。

キーが複数段になる場合、 `std::vector<std::string>` を渡すことができます。

```cpp
#include <toml.hpp>
#include <cassert>

int main()
{
    const toml::value v(toml::table{
        {"a", 42},
        {"b", "foo"},
    });
    const std::string s = toml::format("bar", v);

    const toml::value u = toml::parse_str(s);

    assert(u.at("bar").at("a").as_integer() == 42);
    assert(u.at("bar").at("b").as_string()  == "foo");

    return 0;
}
```

## フォーマットを指定する

`toml::value` のそれぞれの型には、対応するフォーマット情報型があります。

`toml::value::integer_type` には `toml::integer_format_info` が、
`toml::value::table_type` には `toml::table_format_info` があります。

これらは、パースした際に設定され、型が変わらない限り値を変更しても引き継がれます。

また、 `as_integer_fmt()` や `as_table_fmt()` といったメンバ関数によってアクセスすることができ、
直接編集することが可能です。

以下ではいくつかの例を挙げて使い方を説明します。

フォーマットへのアクセス方法は
[`toml::value`のリファレンス]({{< ref "/docs/reference/value" >}})を、
フォーマット情報クラスの完全なリストと詳細は
[formatのリファレンス]({{< ref "/docs/reference/format" >}})を
参照してください。

### 整数のフォーマットを指定する

整数は、基数と幅、そして `_` の位置を指定することができます。

`hex`, `oct`, `bin` のとき、指定された幅に達するまでゼロで埋められます。
`dec` の場合は幅指定はスペースを追加しますが、これはパースされません。

```cpp
#include <toml.hpp>

int main()
{
    toml::value v(0x00C0'FFEE);
    v.as_integer_fmt().fmt    = toml::integer_format::hex;
    v.as_integer_fmt().width  = 8;
    v.as_integer_fmt().spacer = 4;

    const std::stirng s = toml::format(v);
    assert(s == "0x00C0_FFEE");

    return 0;
}
```

詳細は、[reference]({{< ref "/docs/reference/format#integer_format" >}}) を参照してください。

### 配列を単一行・複数行にする

配列には、 `toml::array_format::oneline` や `toml::array_format::multiline` を指定できます。

```toml
# oneline
a = [1, 2, 3, 4, 5]
# multiline
a = [
  1,
  2,
  3,
  4, 
  5
]
```

`multiline` のときは、インデントを指定できます。
各要素は `body_indent` の分だけインデントされ、閉じ括弧 `]` は `closing_indent` の分だけインデントされます。

文字種は `indent_type` で指定され、 `toml::indent_char::space` または `toml::indent_char::tab` が選択できます。

{{<hint warning>}}
インデントに使用する文字種は統一してください。

一つのファイル内でインデントに異なる文字種が指定された場合、結果は未規定になります。
何らかのインデントがされますが、全ての箇所で文字種やインデントの深さは不定となります。
{{</hint>}}


また、 `array` の要素が全て `table_type` を持つ場合、
`toml::array_format::array_of_tables` が指定できます。

`array_of_tables` を指定せずに `multiline` にした場合、テーブルはインラインテーブルになります。

```toml
# multiline
a = [
  {foo = 42},
  {bar = "hoge"},
]

# array_of_tables
[[a]]
foo = 42

[[a]]
bar = "hoge"
```

デフォルトでは、 `toml::array_format::default_format` が指定されます。
これは適したフォーマットを自動的に選択します。

例えば、 `default_format` で全要素が `table_type` だった場合、 `array_of_tables` が選択されます。
また、十分短い配列は `oneline` に、長い配列またはネストされた配列などの複雑な配列は `multiline` になります。

詳細は、[reference]({{< ref "/docs/reference/format#array_format" >}}) を参照してください。

### テーブルをインラインテーブルにする

テーブルをインラインテーブルにする際は `toml::table_format::oneline` を指定します。
通常のテーブルにする際は、 `toml::table_format::multiline` を指定します。

```toml
oneline = {a = 42, b = "foo"}

[multiline]
a = 42
b = "foo"
```

TOML v1.1.0ではインラインテーブル内での改行が許可されますが、その場合は
`toml::table_format::multiline_oneline` とします。
これは、後述するTOMLバージョン指定で対応する機能フラグが`true`になっていない限り無視されます。

```toml
multiline_oneline = {
    a = 42,
    b = "foo"
}
```

詳細は、[reference]({{< ref "/docs/reference/format#table_format" >}}) を参照してください。

## TOML言語バージョンを指定して出力する

TOML v1.1.0で許可されたインラインテーブル内の改行や`\x`エスケープシーケンスのように、
TOMLバージョンによって使用できない言語機能があります。

`toml::format` は最後の引数に `toml::spec` を取ることができます。

これにより、シリアライズ時に使用するTOMLのバージョンを指定することができます。

特に、 `toml::parse` で `toml::spec` を使用して新機能を使用した場合は、
パースした値がそのバージョンでしか使えないフォーマット情報を持つ場合があるので、
`toml::format` にも同じ `toml::spec` を渡すことを忘れないようにしてください。

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    const auto spec = toml::spec::v(1, 1, 0)

    const toml::value v = toml::parse("input.toml", spec);

    std::cout << toml::format(v, spec);

    return 0;
}
```

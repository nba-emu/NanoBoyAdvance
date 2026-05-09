+++
title = "extension"
type  = "docs"
weight = 80
+++

# TOML言語拡張

TOML言語は現在 v1.0.0 が最新版ですが、その後もいくつかの新機能が議論の末マージされ、
v1.1.0に向けて議論が続いています。

そこで議論された機能の中には、有用なケースが少ないと考えられたものや、
提案された際の方向性では導入が難しいもの、導入がされなかったものも多くあります。

toml11では、そのような機能のなかからいくつかを選んで、実験的に実装を行っています。
これらはtoml11ではサポートされていますが、他のパーサではサポートされておらず、また
サポートされる予定もないことに注意してください。

また、これらの機能はデフォルトで使用されない設定になっており、
使用するためには機能フラグをそれぞれ `true` にしなければなりません。
非標準の機能なので、あえて明示的に書かなければ使えないように設計しています。

いくつかの機能は今後TOML言語自体に新機能としてマージされる可能性があります。
もし以下の拡張機能を完全に置き換えられる機能が導入された場合、拡張機能は
本来の機能の実装後にマイナーバージョンアップで削除される可能性があります。

## `null`

TOMLファイル内で値として`null`を使えるようになります。

```
a = null
b = [ 1, 2, 3, null, 5]
```

これを使用するには、 `toml::spec` の `ext_null_value` を `true` にします。

パースすると、デフォルト構築した場合と同様の `toml::value_t::empty` となります。
ただし、ファイル内の位置情報は設定されます。

`null` は値の文脈でのみパースされるので、キーに `null` を使用した際はこれまで通り
`"null"` という文字列のキーとして解釈されます。

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec;
    spec.ext_null_value = true;

    const auto v = toml::parse_str("a = null", spec);

    assert(v.at("a").is_empty());
    assert(v.at("a").is(toml::value_t::empty));

    return 0;
}
```

## 浮動小数点数の16進数フォーマット

TOMLファイル内で浮動小数点数に16進数フォーマットを使用できるようになります。

```
a = 0x1.91eb851eb851fp+1 # 3.14
```

これを使用するには、 `toml::spec` の `ext_hex_float` を `true` にします。

フォーマットは `printf` で `%a/%A` を指定した場合に準拠します。

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec;
    spec.ext_hex_float = true;

    const auto v = toml::parse_str("a = 0x1.91eb851eb851fp+1", spec);

    assert(v.at("a").is_floating());
    assert(v.at("a").as_floating() == 3.14);

    return 0;
}
```

## 整数・浮動小数点数のsuffix

TOMLファイル内で数値の後ろにsuffixをつけられるようになります。
10進数表記の整数と浮動小数点数で使用できます。

単位を表示するときなどに便利です。

```
a = 86_400_sec
b = 3.1416_rad
c = 10_μm
```

ですが、これらはあくまで単なる `suffix` であり、単位換算は行われません。
単位換算が必要な場合は、ユーザーが `suffix` を参照して実装してください。

これを使用するには、 `toml::spec` の `ext_num_suffix` を `true` にします。

数値と接尾辞の間は`_`で区切られている必要があります。

数値部分との区別のため、suffixは数値で始まることはできません。

```
distance = 100_m  # valid
distance = 10_0m  # invalid
distance = 10_0_m # valid
```

接尾辞は`std::string suffix`としてフォーマット情報に保持されます。

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec;
    spec.ext_hex_float = true;

    const auto v = toml::parse_str("a = 86_400_sec", spec);

    assert(v.at("a").is_integer());
    assert(v.at("a").as_integer() == 86400);
    assert(v.at("a").as_integer_fmt().suffix == "sec");

    return 0;
}
```

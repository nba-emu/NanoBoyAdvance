+++
title = "error message"
type  = "docs"
weight = 30
+++

# エラーメッセージを出力する

toml11は `toml::parse` や `toml::get<T>/find<T>`, `as_integer()` などから
ファイル内の位置情報を含んだエラーメッセージを出力します。

例えば、パース時に整数の文法エラーを発見した場合、

```
[error] bad integer: `_` must be surrounded by digits
 --> internal string at line 64 in file main.cpp
   |
 1 | a = 123__456
   |        ^-- invalid underscore
Hint: valid  : -42, 1_000, 1_2_3_4_5, 0xC0FFEE, 0b0010, 0o755
Hint: invalid: _42, 1__000, 0123
```

あるいは実際に格納されている型と異なる型を要求した場合

```
[error] toml::value::as_string(): bad_cast to string
 --> input.toml
   |
 1 | a = 123_456
   |     ^^^^^^^-- the actual type is integer
```

toml11は `toml::value` からこのようなエラーメッセージを作成する方法を提供します。

この機能を利用することで、TOMLの文法エラーだけでなく、
例えば正の値でなければならないところに負数が現れた場合などの
アプリケーション固有のエラーメッセージを、TOMLファイル内の位置を指摘しながら
ユーザーに伝えられるということです。

## `toml::value` の位置情報からエラーメッセージを作成する

`toml::value` はそれがパースされた位置の情報を持っています。

その情報は `toml::source_location` にまとめられ、`toml::value::location()` で取得できます。

```cpp
const toml::value& a = input.at("a");
const toml::source_location src = a.location();
```

ファイルを `toml::parse` でパースした場合、そのTOMLファイル名と行数が保存されています。

`toml::parse_str` でパースした場合TOMLファイル名はありませんが、代わりに
`toml::parse_str` を呼び出したC++ソースコードのファイル名と行数がTOMLファイル名として保存されています。
このページの最初の例は `toml::parse_str` から出力された例でした。
ファイル名の部分に注目してください。

詳細は [reference]({{<ref "/docs/reference/source_location">}}) を参照してください。

`toml::source_location` または `toml::value` とそれに付随するエラーメッセージを
`toml::make_error_info` に渡すことで、エラー情報を構築できます。
これを`toml::format_error` に渡すと、エラーメッセージが `std::string` にフォーマットされます。

```cpp
const toml::value& a = input.at("a");
if(a.as_integer() < 0)
{
    const toml::error_info err = toml::make_error_info(
            "positive integer is required", // エラーのタイトル
            a, "but got negative value"     // 値の横に書くメッセージ
        );
    std::cerr << toml::format_error(err) << std::endl;
}
```

これは以下のようになります。

```
[error] positive integer is required
 --> input.toml
   |
 1 | a = -123456
   |     ^^^^^^^-- but got negative value
```

最後に補足をつけ足すこともできます。これはインデントされません。

```cpp
const toml::value& a = input.at("a");
if(a.as_integer() < 0)
{
    const toml::error_info err = toml::make_error_info(
            "positive integer is required",      // エラーのタイトル
            a, "but got negative value",         // 値の横に書くメッセージ
            "Hint: `a` means length of the data" // 補足
        );
    std::cerr << toml::format_error(err) << std::endl;
}
```

```
[error] positive integer is required
 --> input.toml
   |
 1 | a = -123456
   |     ^^^^^^^-- but got negative value
Hint: `a` means length of the data
```

{{<hint info>}}
`toml::value` からファイル内の行を出力できるのは、
パースしたファイルが文字列としてメモリの中に残されているからです。

パースした文字列はその全体が `std::shared_ptr` で `toml::value` に共有されています。
コピーしてもファイル文字列全体がコピーされることはありません。
また、そのファイルをパースして構築された `toml::value` が全てデストラクトされた時点で、
ファイル情報もメモリ上から解放されます。

ですので、アプリケーションで使用する際には、 `toml::value` を直接保存するのではなく
読み込み中に必要な値を全て取り出して、変換した値を保存した方がよいでしょう。
{{</hint>}}

## 文字列に色を付ける

エラーメッセージにはANSIエスケープコードを使って色を付けることができます。

`TOML11_COLORIZE_ERROR_MESSAGE` をコンパイル時に定義していれば、
toml11の出力するエラーメッセージはデフォルトで色が付くようになります。

そうでない場合は、 `toml::color::enable()` を呼び出すことにより、それ以降で出力される
エラーメッセージには色が付くようになります。
逆に出力先がコンソールではないなどの理由で色をつけたくない場合は、
`toml::color::disable()` を呼び出してください。
その時点で色が付くようになっているかどうかは、
`toml::color::should_color()` の返り値で判定できます。

詳細は [reference]({{<ref "docs/reference/color">}}) を参照してください。

また、エラーのタイトルやエラーメッセージ、補足にはデフォルトで色が付きませんが、
`toml::color` にあるマニピュレータを使って色を付けることも可能です。

```cpp
std::ostringstream oss;
oss << toml::color::red << "but got negative value";

const toml::error_info err = toml::make_error_info(
        "positive integer is required",      // Error title
        a, oss.str(),                        // Message next to the value
        "Hint: `a` means length of the data" // Supplementary message
    );
```

こちらも、詳細は [reference]({{<ref "docs/reference/color">}}) を参照してください。

## エラーメッセージのprefixを`[error]`から変更する

エラーには種類があり、デフォルトの `[error]` ではよくない場合もあるでしょう。

`toml::format_error` では、 `toml::error_info` の前に `std::string` を取って、それを
`[error]` の代わりに出力することができます。

例えば、

```cpp
const toml::value& a = input.at("a");
if(a.as_integer() < 0)
{
    const toml::error_info err = toml::make_error_info(
            "positive integer is required",      // エラーのタイトル
            a, "but got negative value"          // 値の横に書くメッセージ
        );

    std::ostringstream prefix;
    prefix << toml::color::bold << toml::color::yellow << "[warn]";
    std::cerr << toml::format_error(prefix.str(), err) << std::endl;
    return 0;
}
else
{
    return a.as_integer()
}
```

このようにすると、 `[warn]` から始まる警告を出力することができます。

他にも、`toml::format_error` に直接 `error_info` の構成要素を渡すことで、
`[error]` なしのエラーメッセージを作成できます。

```cpp
const toml::value& a = input.at("a");
if(a.as_integer() < 0)
{
    std::cerr << toml::format_error(
            "[warn] positive integer is required",      // エラーのタイトル
            a, "but got negative value"          // 値の横に書くメッセージ
        ) << std::endl;
    return 0;
}
else
{
    return a.as_integer()
}
```

## 複数の `toml::value` を指すエラーメッセージを作成する

アプリケーションの設定では、先に読み込んだ値によって後に読み込んだ値が取れる範囲が変わることがあるでしょう。

そのような場合には、エラーの原因となる別の値を同時に出力したいはずです。

`toml::format_error` と `toml::make_error_info` は、 `toml::value` とそれに対応するエラーメッセージ `std::string` のペアを任意個取ることができます。

```cpp
std::cerr << toml::format_error(
        "[error] invalid range",
        a, "minimum value is defined here",
        b, "maximum value is defined here",
        c, "and it exceeds the range"
    ) << std::endl;
```

こちらも末尾に補足を追加することができます。

```cpp
std::cerr << toml::format_error(
        "[error] invalid range",
        a, "minimum value is defined here",
        b, "maximum value is defined here",
        c, "and it exceeds the range",
        "Hint: all the value must be in the range, [a, b)"
    ) << std::endl;
```

`toml::value` または `toml::source_location` を渡した場合、必ずそれに関する
エラーメッセージが続く必要があります。
そうしない場合、非常にわかりにくいコンパイルエラーになります。

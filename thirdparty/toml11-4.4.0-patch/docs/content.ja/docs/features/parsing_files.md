+++
title = "parsing files"
type  = "docs"
weight = 10
+++

# ファイル・文字列をパースする

toml11では、`toml::parse` や `toml::try_parse` を使って、ファイルや文字列、バイト列をパースすることができます。

これらは成功時に `toml::value` を返します。
ファイルは常にテーブルになりますが、返り値が `toml::table` でないことに気を付けてください。
`toml::value` はファイルに関するメタデータを持っており、
`toml::table` は `std::unordered_map<std::stirng, toml::value>` のエイリアスでしかありません。
メタデータを返すために、 `toml::table` ではなく `toml::value` を返しています。
ファイルのルートに対応する `toml::value` は常に `table_type` を持ちます。

## ファイルをパースする

ファイルをパースする際は、
[`toml::parse`]({{< ref "docs/reference/parser#parse" >}})
または
[`toml::try_parse`]({{< ref "docs/reference/parser#try_parse" >}})
を使います。

### `toml::parse`

#### `std::string`でファイル名を指定する

[`toml::parse`]({{< ref "docs/reference/parser#parse" >}})
は、文字列でファイル名を受け取り、そのファイルを開いてパースします。

以下のサンプルは、`input.toml`というファイルをパースし、`title`という変数を文字列として取り出し、出力するコードです。

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    const toml::value input = toml::parse("input.toml");
    std::cout << input.at("title").as_string() << std::endl;
    return 0;
}
```

#### `std::filesystem::path`でファイルを指定する

[`toml::parse`]({{< ref "docs/reference/parser#parse" >}})
には、`std::filesystem::path`を渡すことも可能です。

当然ですが、`<filesystem>`がサポートされるC++17以降でなければ使用できません。

#### `std::istream`で入力ストリームを指定する

[`toml::parse`]({{< ref "docs/reference/parser#parse" >}})
には、`std::istream`を渡すことも可能です。

標準ライブラリが改行文字を自動変換することによるファイルサイズと文字数との不整合を避けるため、
`std::ios::binary`を使ってバイナリモードで開いてください。

その際、ファイル名の情報がなくなるため、エラーメッセージ中では `"unknown file"` となります。

これを避けるため、 `std::istream` を取る場合は第二引数に `std::string` でファイル名を取ることもできます。

`std::ifstream` 以外にも、 `std::istringstream` 等の別の`istream`を受け取ることができます。
ただし、呼び出した時点で内容が全て読み込める必要があります。

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    std::string filename("input.toml");
    std::ifstream ifs(filename);
    const toml::value input = toml::parse(ifs, filename);
    std::cout << input.at("title").as_string() << std::endl;
    return 0;
}
```

#### `FILE*`でファイルを指定する

[`toml::parse`]({{< ref "docs/reference/parser#parse" >}})
には、`FILE*`を渡すことも可能です。

標準ライブラリによる改行文字の自動変換によるファイルサイズと文字数との不整合を避けるため、
`fopen("example.toml", "rb")`のようにしてバイナリモードで開いてください。

この場合も、`std::istream`のときと同様に、第二引数に文字列でファイル名を与える必要があります。

`FILE*`を渡した場合、ファイルの読み込みに失敗した際には`errno`が報告されます。

#### エラー時の挙動

[`toml::parse`]({{< ref "docs/reference/parser#parse" >}})
は、文法エラーを発見した場合
[`toml::syntax_error`]({{<ref "docs/reference/parser#syntax_error">}})
を送出します。

[`toml::syntax_error`]({{<ref "docs/reference/parser#syntax_error">}})
は、
[`toml::exception`]({{<ref "docs/reference/exception">}})
から派生しており、またそれは`std::exception`から派生します。

よって、
[`toml::syntax_error`]({{<ref "docs/reference/parser#syntax_error">}})
からは `what()` メンバ関数を使ってエラーメッセージを取り出すことができます。

また、
[`toml::syntax_error`]({{<ref "docs/reference/parser#syntax_error">}})
は
[`std::vector<toml::error_info>`]({{<ref "docs/reference/error_info">}})
を持っており、`errors()`メンバ関数を使ってそれにアクセスすることもできます。

`toml::parse` はごく簡単なエラーならスキップして復帰し、複数のエラーを報告しようと努めます。
数値のフォーマットエラー程度なら復帰できることが多いですが、
配列やテーブルの中のエラーは復帰できずに似たようなエラーを複数回報告することもあります。
冗長であると感じた場合は、 `std::vector<toml::error_info>` の `front()` だけを使用するようにすると、
確実に問題のある個所に関するメッセージを得られます。

```cpp
#include <toml.hpp>

int main()
{
    // parse
    try {
        const toml::value input = toml::parse("input.toml");
        std::cout << input.at("title").as_string() << std::endl;
    } catch(const toml::syntax_error& err) {
        // 全てのエラーを報告
        std::cerr << err.what() << std::endl;

        // 最初のエラーのみ報告
        std::cerr << err.errors().front() << std::endl;
    }
}
```

### `toml::try_parse`

`toml::parse` は失敗時に例外を送出しますが、 `toml::try_parse` は失敗時に例外を投げません。

その代わり、返り値が `toml::value` ではなく [`toml::result<toml::value, std::vector<toml::error_info>>`]({{<ref "docs/reference/result#result">}}) になります。

[`result`]({{<ref "docs/reference/result#result">}}) 型は成功値または失敗値のどちらかを持つ型です。
Rustの `Result` やHaskellの `Either` に相当します。

```cpp
#include <toml.hpp>

int main()
{
    const auto parse_result = toml::try_parse("input.toml");
    if(parse_result.is_ok())
    {
        std::cout << parse_result.unwrap().at("title").as_string() << std::endl;
    }
    else
    {
        std::cerr << parse_result.unwrap_err().at(0) << std::endl;
    }
    return 0;
}
```

[`result`]({{<ref "docs/reference/result#result">}}) 型がどちらの値を保持しているかは
`is_ok()`, `is_err()` 関数を使って確認できます。
また、 `unwrap()`, `unwrap_err()` によって成功値、失敗値をそれぞれ取り出せます。
`unwrap` が失敗した場合は、 `bad_result_access` 例外が送出されます。
`as_ok()` と `as_err()` 関数を使用すると、失敗時には例外が送出されず、未定義動作となります。

{{<hint warning>}}

`try_parse` は `syntax_error` や `file_io_error` を投げず、同じ `toml::error_info` を
`result` の失敗型として返しますが、絶対に例外を投げないわけではありません。

標準ライブラリ内部でエラーが発生した場合、例えばメモリ不足の際に `vector` の
`allocate` が失敗した場合などには `std::bad_alloc` が送出されますが、
`toml::try_parse` はこれが送出された場合は `catch` せずに通します。
そのため、それらのような標準ライブラリの内部で発生する例外は送出される恐れがあります。

{{</hint>}}

## 文字列をパースする

### `toml::parse_str`

[`toml::parse_str`]({{<ref "docs/reference/parser#parse_str">}})
は、ファイル名ではなくパースする文字列そのものを受け取ります。

また、エラーメッセージのTOMLファイルの名前に相当する部分には、`std::source_location`
相当のコンパイラ拡張機能が使用できる場合、 `parse_str` を呼び出したC++ファイルの名前と行数が代わりに使用されます。

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    const toml::value input = toml::parse_str("title = \"parse_str\"");
    std::cout << input.at("title").as_string() << std::endl;
    return 0;
}
```

### `toml::try_parse_str`

[`toml::try_parse_str`]({{<ref "docs/reference/parser#try_parse_str">}})
は、 `parse_str` と同じくパースする文字列そのものを受け取り、
`try_parse` と同じくエラーの報告に
[`toml::result`]({{<ref "docs/reference/result#result">}})
を使用します。

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    const auto parse_result = toml::try_parse_str("title = \"parse_str\"");
    if(parse_result.is_ok())
    {
        std::cout << parse_result.unwrap().at("title").as_string() << std::endl;
    }
    else
    {
        std::cerr << parse_result.unwrap_err().at(0) << std::endl;
    }
    return 0;
}
```

## バイト列をパースする

ファイルではなくバイト列をパースすることも可能です。

UTF-8でエンコードされている必要があるため、`unsigned char`を使っています。

### `toml::parse(std::vector<unsigned char>)`

挙動は [`toml::parse`]({{<ref "docs/reference/parser#parse">}}) と同一です。

バイト列をパースする際は、 `filename` を要求します。

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    std::vector<unsigned char> bytes{/* ... */};
    const toml::value input = toml::parse(bytes, "internal bytes");
    std::cout << input.at("title").as_string() << std::endl;
    return 0;
}
```

### `toml::try_parse(std::vector<unsigned char>)`

挙動は [`toml::try_parse`]({{<ref "docs/reference/parser#try_parse">}}) と同一です。

バイト列をパースする際は、 `filename` を要求します。

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    std::vector<unsigned char> bytes{/* ... */};
    const auto parse_result = toml::try_parse(bytes, "internal bytes");
    if(parse_result.is_ok())
    {
        std::cout << parse_result.unwrap().at("title").as_string() << std::endl;
    }
    else
    {
        std::cerr << parse_result.unwrap_err().at(0) << std::endl;
    }
    return 0;
}
```

+++
title = "changelog"
type  = "docs"
weight = 4
+++

# Change Log

# v4.4.0

## Added

- `toml::find_or_default()` を追加 (by Ken Matsui @ken-matsui)
- `ordered_map`から値を削除できるよう変更 (by Sunlight @SunPodder)
- 値がアクセス済みかを調べる`bool basic_value::accessed() const`を追加
  - `TOML11_ENABLE_ACCESS_CHECK`が定義されているときのみ有効
- `toml::spec`に比較演算子を追加

## Fixed

- `is_same<T, void>`ではなく`is_void`を使用するように修正 (by Ken Matsui @ken-matsui)

## Changed

- `toml::parse`のパフォーマンスを約2倍向上
- GitHub ActionsでUbuntu 20のイメージの使用を終了

# v4.3.0

## Added

- `toml::find`で`std::optional<T>`をサポート
- `toml::visit`で複数の引数をサポート

## Fixed

- 初期化前に変数が使われるケースを修正 (by amatej @kontura)
- CMake 3.21以前を使用する際の修正 (by Severin Leonhardt @SeverinLeonhardt)
- 非常に巨大な文字列が作成されるケースを修正 (by @hayt)
- READMEのサンプルコードを修正、ToCを増強 (by somebody @oldoldtea) (by lz)
- 長い行のパースを高速化
- MSVC 2017でのコンパイルエラーを修正
- `source_location::file_name`のヌルチェックを追加

## Changed

- hugo-bookテーマをアップデート
- MSVC 2017をappveyor buildに追加

# v4.2.0

## Added

- `TOML11_DEFINE_CONVERSION_NON_INTRUSIVE` で `std::optional` なメンバをサポート (by Ken Matsui @ken-matsui)
- `thread_local`だった`color_mode`をデフォルトでグローバルにし、`thread_local`にするオプションを追加 (by Ken Matsui @ken-matsui)
- CPMでの使い方を`README`に追加
- `README`に`ordered_map`への言及を追加し、ドキュメントでの説明を追加

## Fixed

- ファイルサイズの`std::streamsize`への変換で警告が出ることがある問題を修正 (by Pino Toscano @pinotree)
- `table_format`に不正な値が与えられた際の出力のtypoを修正
- `array`のフォーマットが`array_of_tables`と指定されていてかつ空の場合の出力を修正
- 特定の環境で`include`が足りずにコンパイルできない問題を修正
- ドキュメントに含まれる文法エラーを修正 (by Jack W)
- `toml::find_or` を深くネストされたテーブルに使用した際にコンパイルが失敗する問題を修正

# v4.1.0

## Added

- `std::get<std::u8string>`をサポート
- `toml::value(std::u8string)`をサポート
- `string_type = std::u8string`をサポート
- `operator<<(std::ostream&, toml::value)`をサポート
- `toml::integer_format`に、16進数表示で大文字を使うことを指定する`bool uppercase`を追加
- `template`を使って実装された `into_toml()` をサポート (by 萧迩珀)

## Fixed

- Windowsで`\r\n`が改行に使われた際、出力時に`\r`が重複する問題を修正 (by Andreas Keller)

## Changed

- CIで複数コアを使うことでビルドを高速化

# v4.0.3

## Fixed

- `toml_fwd.hpp`を使用した際にデフォルト引数が重複する問題を修正
- `toml::value`を複数渡した際に`toml::make_error_info`が使えない問題を修正
- `std::reference_wrapper`を持つ`toml::result`をコピー・ムーブする際の問題を修正
- hugoの最新版でドキュメントがビルドできない問題を修正
- コンパイラによる軽微な警告を多数修正
- CMakeの互換性に関する警告を抑制

## Changed

- CIビルドで`-Werror`, `/WX`を追加
- CIビルドでMSVCのwarningレベルを`/W3`から`/W4`に変更
- READMEでより多くの機能を紹介するよう更新

# v4.0.2

## Fixed

- `parse(FILE*)` 内の `fread` の結果をチェック
- `toml11/version.hpp`のマクロを修正
- コンパイルオプションに関するドキュメントの更新
- ファイルオープンモードに関するドキュメントの更新

## Changed

- `CMakeLists.txt`内のバージョン番号を`version.hpp`から取得するように変更

# v4.0.1

## Fixed

- `sematic_version::{major, minor}` と `<sys/sysmacro.h>` 内で定義されるマクロの衝突を解消
- `discard_comments` の `operator<<` の定義を修正 (by Egor Pugin)
- `format_location`を使用した際に最初の空行が出ない問題を解決
- 改行文字のみを含む行を指す`source_location`でエラーメッセージを生成した際に、同じ行が二回表示される問題を解決
- `README.md`内のリンクを修正
- `example/unicode`のREADMEのタイトルを修正

## Added

- `main`に変更があったとき、`single_include/toml.hpp`を自動生成するようCIを設定

# v3からv4への変化

## 破壊的変更

### `toml::basic_value`の`template`引数を変更

toml11 v3では、`toml::basic_value`はコメントコンテナ、テーブル型コンテナ、配列型コンテナをそれぞれ取っていました。

```cpp
template<typename Comment,
         template<typename ...> class Table = std::unordered_map,
         template<typename ...> class Array = std::vector>
class basic_value;
```

ですが、`integer_type`などを変更したいという場合にはこれでは対応できません。

toml11 v4では、`toml::basic_value`は単一の`TypeConfig`を受け取り、より多くの型を変更可能にします。

```cpp
template<typename TypeConfig>
class basic_value;
```

デフォルトでは`toml::value`が格納する型は変わりません。

型を変更する際は、
[`type_config`]({{< ref "/docs/reference/types.md">}}) 
を参照してください。

### `toml::basic_value`の`std::initializer_list`サポートを削除

toml11 v3では、 `toml::value` に `std::initializer_list` を取るオーバーロードが用意されていました。

これにより、 `toml::value` を配列やテーブルで初期化する際により直観的な書き方が可能でした。

```cpp
// toml11 v3
toml::value v{1,2,3,4,5};
toml::value v{ {"a", 42}, {"b", "foo"} };
```

しかし、これは同時に以下のような問題を引き起こしました。

一つ目は、1要素の配列と通常の値の区別がつかず、常に配列になってしまうことです。

```cpp
// toml11 v3
toml::value v{1}; // 1 ではなく [1,] になってしまう
```

統一初期化記法が普及した現在、これは非常に不便です。

二つ目は、値が全て文字列のテーブルと、ネストされた配列の区別がつかないことです。

```cpp
// toml11 v3
toml::value v{ {"a", "foo"}, {"b", "bar"} };
// {a = "foo", b = "bar"}
// [["a", "foo"], ["b", "bar"]]
// のどちらでもあり得る
```

これらの問題は言語仕様上解決が困難です。

toml11 v4では、混乱を避けるため、`std::initializer_list`サポートを削除しました。

`toml::value` を配列で初期化する際はexplicitに `toml::array` を、
テーブルで初期化する際はexplicitに `toml::table` を指定する必要があります。

```cpp
// toml11 v4
toml::value v(toml::array{1,2,3,4,5});
toml::value v(toml::table{ {"a", 42}, {"b", "foo"} });

toml::value v{toml::array{1}}; // [1,]
toml::value v{1}               // 1

toml::value v{toml::table{{"a", "foo"}, {"b", "bar"}}};
toml::value v{toml::array{toml::array{"a", "foo"}, toml::array{"b", "bar"}}};
```

これにより `toml::value` をテーブルや配列で初期化する際に少し不便になりますが、
explicitに型情報を記述することにより予測不能な値になることは避けることができます。

### `toml::basic_value::is_uninitialized()` を `is_empty()` に変更

toml11 v3では、初期化されていない `basic_value` かどうかを判定する関数は
`is_uninitialized` でした。

しかし、toml11 v4では言語拡張で `null `をサポートするため、意図的に空にされた値が構築される可能性があります。
なので命名を変更し、 `is_empty` としました。

### `toml::string` を廃止、フォーマット情報を陽に格納

toml11 v3では、文字列が `basic` か `literal` かの情報を保持するため、
格納する文字列型に `toml::string` という `std::string` の薄いラッパーを使用していました。

```cpp
// toml11 v3
namespace toml
{
enum class string_t : std::uint8_t
{
    basic   = 0,
    literal = 1,
};

struct string
{
    string_t    kind;
    std::string str;
};
} // toml
```

toml11 v4では、数値型の基数や配列を複数行にするかどうかなどのより多くのフォーマット情報を持たせるため、
全ての型に `xxx_format` 型を用意し、値とペアで格納することにしました。

```cpp
// toml11 v4
enum class string_format : std::uint8_t
{
    basic             = 0,
    literal           = 1,
    multiline_basic   = 2,
    multiline_literal = 3
};

struct string_format_info
{
    string_format fmt = string_format::basic;
    bool start_with_newline    = false;
};
```

これにより、より細かいフォーマット情報を保持することができるようになり、
特に数値型や配列、テーブル型のフォーマット情報がパース後も維持できるようになりました。

### `toml::format`の引数を変更

toml11 v3では、 `toml::format` が数値型の精度や幅などの値を受け取っていました。

しかし、これでは細かなフォーマット指定ができず、予想したようなファイルにシリアライズできませんでした。

toml11 v4では、各 `toml::value` にそれぞれのフォーマット情報を格納したため、
より細かいフォーマット情報を `toml::value` 自体が持ち運べるようになりました。

これにより、 `toml::format` 自体はフォーマット指定を受け取らず、
フォーマット時に使用できる言語機能フラグを持つ `toml::spec` のみを取るようになりました。

### `toml::source_location`のメンバ関数を変更

toml11 v3では、 `toml::source_location` のメンバ型は一行分だけを意識したものでした。

toml11 v4では、 `toml::source_location` のメンバ型は複数行が前提になります。

### `toml::format_underline`を`toml::format_location`に変更

toml11 v3では、 `toml::source_location` を使用して位置情報を文字列化するとき、
`toml::format_underline` を使用していました。

しかしこれは名称としてわかりにくいため、 `toml::format_location` に変更しました。

### `format_error`の引数を変更

toml11 v3ではエラー情報を表すクラスがなかったため、`toml::format_error`の引数が複雑になっていました。

```cpp
template<typename C, template<typename ...> class T, template<typename ...> class A>
std::string format_error(const std::string& err_msg,
        const basic_value<C, T, A>& v, const std::string& comment,
        std::vector<std::string> hints = {},
        const bool colorize = TOML11_ERROR_MESSAGE_COLORIZED);

template<typename C, template<typename ...> class T, template<typename ...> class A>
inline std::string format_error(const std::string& err_msg,
        const toml::basic_value<C, T, A>& v1, const std::string& comment1,
        const toml::basic_value<C, T, A>& v2, const std::string& comment2,
        std::vector<std::string> hints = {},
        const bool colorize = TOML11_ERROR_MESSAGE_COLORIZED);

template<typename C, template<typename ...> class T, template<typename ...> class A>
inline std::string format_error(const std::string& err_msg,
        const toml::basic_value<C, T, A>& v1, const std::string& comment1,
        const toml::basic_value<C, T, A>& v2, const std::string& comment2,
        const toml::basic_value<C, T, A>& v3, const std::string& comment3,
        std::vector<std::string> hints = {},
        const bool colorize = TOML11_ERROR_MESSAGE_COLORIZED);
```

toml11 v4では、`class error_info`と`make_error_info`を導入し、`format_error`の引数を簡略化しました。

```cpp
std::string format_error(const error_info& err);
std::string format_error(const std::string& errkind, const error_info& err);

template<typename ... Ts>
std::string format_error(std::string title,
        source_location loc, std::string msg, Ts&& ... tail);

template<typename TC, typename ... Ts>
std::string format_error(std::string title,
        const basic_value<TC>& v, std::string msg, Ts&& ... tail);
```

### `toml::color` の制御を変更

toml11 v3では、出力に色を付けるかどうかに `toml::colorize` というマニピュレータを、
`toml::color::enable/disable` と並列で使用していました。

マニピュレータはストリームごとに色を付けるかどうかを決定できるものでしたが、
v4でストリームを使用する頻度が下がったことと、内部で使用する
`std::ios_base::xalloc` がC++11ではスレッドセーフではないことなどから、
`toml::color::enable/disable` のみを使用するようにし、 `toml::colorize` を削除しました。

## 破壊的でない変更

### `parse_str`の追加

toml11 v3では、文字列そのものを取る`toml::parse`関数はありませんでした。
なので文字列をパースする際には`std::istringstream`を使う必要がありました。

これは不便なので、`toml::parse_str`を追加し、文字列を直接パース出来るようにしました。

### `try_parse`の追加

toml11 v3では、パーサはエラーを発見した際には`toml::syntax_error`を送出していました。

しかし、例外を投げられない環境や、パフォーマンスの都合や記法の都合によって例外を投げたくない場合があります。

toml11 v4では `toml::result` を使用して、例外を投げずにパース失敗を伝える `toml::try_parse` を実装しました。

これも必ず例外を投げないというわけではなく、使用している標準ライブラリの内部でのエラー、
例えばメモリ不足によるアロケーション失敗での `std::bad_alloc` などは送出される可能性があります。

### バイト列のパースをサポート

ファイルなど以外の方法で得られたTOMLコンテンツをパース出来るようにするため、
`std::vector<unsigned char>` を受け取る `toml::parse`, `toml::try_parse` を追加しました。

### `toml::spec`の追加

toml11 v3では、TOML言語側の新機能は全て取り込み、またTOML言語の新バージョンに導入されることが決定した機能も
マクロ `TOML11_USE_UNRELEASED_TOML_FEATURES` によって制御していました。

これは、toml11 v3を開発していた時点では TOML言語は0.4.0から0.5.0で、1.0.0に到達していなかったためです。

最新のTOML言語仕様に全てのユーザーが詳しいわけではないため、
古い言語使用に基づいたエラーメッセージを表示すると、コミュニティ全体を混乱させてしまいます。
よって、v1.0.0に達するまでは、できる限り素早く新しい言語仕様を提供し、かつユーザーにもアップデートを促す必要がありました。

しかし、現在のTOML言語仕様はv1.0.0です。
そのため、TOML v1.1.0がリリースされた後でもv1.0.0を使用する、という選択肢に気を配る必要が生じました。

よって、より柔軟にTOML言語仕様を選択できるよう、`toml::spec`を導入して、TOML言語バージョンを実行時に変更できるようにしました。

また、`toml::spec`では言語機能ごとにフラグを設定し、特定の言語機能のみを試すこともできるようにしました。

これは、toml11 v4特有の言語拡張でも使用します。

### フォーマット情報の追加

toml11 v3では、文字列を除いてフォーマット情報が保存されず、シリアライズ時に考慮できるのは幅と精度だけでした。

しかし、これでは16進数整数がシリアライズ時に10進数になってしまったり、確実にinline tableにする方法がありませんでした。

toml11 v4では、全てのTOML型にフォーマット情報（`integer_format` etc）を追加し、
パース時とシリアライズ時に考慮するようにしました。

これによって、16進数整数や、インラインテーブル等のフォーマット情報をより細かく、値ごとに設定できるようになりました。

### デフォルトでコメントを維持するよう変更

toml11 v3では、デフォルトではコメントがパースされず、シリアライズもされませんでした。

これは、コメントが後期に導入された機能で、特別なハックによって読み込まれていたためです。

toml11 v4では、デフォルトでコメントをパースし、保存し、シリアライズするようになりました。

また、パーサの実装も大幅に変更され、コメントが他の要素と同様にパースされるようになりました。

### `single_include/toml.hpp` の追加

toml11は多機能なライブラリなので、開発効率のために機能ごとに異なるヘッダファイルを持っています。
ですが、これはインストールにある程度の手間が必要ということでもあります。

そこで、toml11 v4から全てのヘッダファイルを適切な順序で結合した `single_include/toml.hpp` ファイルを追加し、
単一のファイルで完結するライブラリをコピーするだけで導入できるようにしました。

### コンパイル済みライブラリを選択可能に

toml11は `template` を多用しているため、コンパイル時間が長くなります。

toml11 v4では、できるだけコンパイル可能な関数を増やし、それらを先にライブラリとしてコンパイルできるようにしました。

これにより、大規模な開発で使用する際にコンパイル時間を短くできると期待できます。

### リファレンス・ドキュメントの追加

これまでは、READMEにすべてのfeatureを記載しており、関数の詳細な定義を示すリファレンスや、また日本語での資料などがありませんでした。

toml11 v4では、リファレンスを追加したドキュメントを同梱し、また日本語・英語両方で同一の内容を記載します。

ただし、ライブラリ作者は日本語母語話者であるため、日本語の内容を第一とし、英語の内容がそれと異なっていた場合は日本語の内容が正しいものとします。

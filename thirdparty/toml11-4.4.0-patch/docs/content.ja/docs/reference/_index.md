+++
title = "reference"
type  = "docs"
weight = 3
bookCollapseSection = true
+++

# Reference

以下では、toml11が公開するクラスと関数の効果を説明します。

## ディレクトリ構造

`toml.hpp` と `toml_fwd.hpp` は `${TOML11_INCLUDE_DIR}` にあります。
他のファイルは、`${TOML11_INCLUDE_DIR}/toml11` にあります。

もし各機能のファイルを個別に `#include` したい場合は、 `#include <toml11/color.hpp>` としてください。
全てを一度に `#include` する場合は、 `#include <toml.hpp>` としてください。

## [color.hpp](color)

エラーメッセージの色付けに関する関数を定義します。

## [comments.hpp](comments)

コメントを持つ`preserve_comment`型と`discard_comment`型を定義します。

## [conversion.hpp](conversion)

`toml::value`とユーザー定義クラスを自動的に変換するマクロを定義します。

## [datetime.hpp](datetime)

日時情報を持つクラスを定義します。

## [error_info.hpp](error_info)

エラー情報を持つクラスを定義します。

## [exception.hpp](exception)

toml11で使用される例外の基底クラス、`toml::exception`を定義します。

## [find.hpp](find)

値を探し変換する`toml::find`関数を定義します。

## [format.hpp](format)

値のフォーマット情報を持つクラスを定義します。

## [from.hpp](from)

ユーザー定義型を変換するための`from<T>`型の前方宣言です。

## [get.hpp](get)

`toml::value`の値を取り出し変換する`toml::get<T>`関数を定義します。

## [into.hpp](into)

ユーザー定義型を変換するための`into<T>`型の前方宣言です。

## [literal.hpp](literal)

`operator"" _toml`リテラルを定義します。

## [ordered_map.hpp](ordered_map)

`toml::ordered_map`を定義します。

## [parser.hpp](parser)

ファイルまたは文字列をパースする関数を定義します。

## [result.hpp](result)

他の関数の返り値として使われる、成功値または失敗値を持つ`result<T, E>`型を定義します。

## [serializer.hpp](serializer)

シリアライズに用いる`toml::format`関数と`toml::serializer`を定義します。

## [source_location.hpp](source_location)

エラー情報に用いられる、ファイル内のある領域を指す`source_location`型を定義します。

## [spec.hpp](spec)

TOML言語のバージョン情報と機能フラグを制御する、`toml::semantic_version`型と`toml::spec`型を定義します。

## [toml.hpp](toml)

`toml.hpp`は、他の全てのヘッダを `include` します。
toml11の全機能が使用可能になります。

## [toml_fwd.hpp](toml_fwd)

`toml_fwd.hpp`は、toml11で定義される構造体の前方宣言と、マクロ定義を持ちます。

## [types.hpp](types)

`toml::value`の持つ型を制御するための`toml::type_config`型を定義します。

## [value.hpp](value)

`toml::value`型を定義します。

## [value_t.hpp](value_t)

列挙型`toml::value_t`を定義します。

## [version.hpp](version)

toml11のバージョン情報を定義します。

## [visit.hpp](visit)

`toml::value`の持つ値に関数を適用する`toml::visit`関数を定義します。

## 備考

ここで明記されない関数（主に`namespace toml::detail`や`namespace toml::cxx`以下に定義されるもの）は、
ソースコードを見ることで利用可能ではあるものの、そのインターフェースは今後のいかなるバージョンアップでも（パッチバージョンアップを含む）維持される保証はありません。

+++
title = "toml_fwd.hpp"
type  = "docs"
+++

# toml_fwd.hpp

`toml_fwd.hpp`は、toml11で定義される構造体の前方宣言と、マクロ定義を持ちます。

toml11の構造体についての前方宣言しか必要なく実装が必要ない場合、
`toml.hpp` のかわりにこちらを `include` することでコンパイル時間を短縮できます。

{{<hint warning>}}

このファイルには前方宣言しか含まれていないため、 
`toml::basic_value<toml::type_config>::table_type` として定義される
`toml::table` と、同様に定義される `toml::array` は使用できません。
それらには `basic_value` の実装が必要だからです。

{{</hint>}}

このヘッダファイルと `toml.hpp` は `${TOML11_INCLUDE_DIR}/` 以下に、
他のヘッダファイルは `${TOML11_INCLUDE_DIR}/toml11/` 以下にあります。


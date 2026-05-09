+++
title = "spec.hpp"
type  = "docs"
+++

# spec.hpp

`spec.hpp`では、TOMLのバージョンを指定するためのクラスが定義されます。

# `toml::semantic_version`

`semantic_version`は、バージョン情報を格納するクラスです。

```cpp
namespace toml
{
struct semantic_version
{
    constexpr semantic_version(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept;

    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
};

constexpr semantic_version
make_semver(std::uint32_t major, std::uint32_t minor, std::uint32_t patch) noexcept;

constexpr bool operator==(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator!=(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator< (const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator<=(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator> (const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator>=(const semantic_version&, const semantic_version&) noexcept;

std::ostream& operator<<(std::ostream& os, const semantic_version& ver);
} //toml
```

## メンバ関数
 
### コンストラクタ

```cpp
constexpr semantic_version(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept;
```

`major`, `minor`, `patch`バージョンを指定して構築します。

## 非メンバ関数
 
### 比較演算子

```cpp
constexpr bool operator==(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator!=(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator< (const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator<=(const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator> (const semantic_version&, const semantic_version&) noexcept;
constexpr bool operator>=(const semantic_version&, const semantic_version&) noexcept;
```

semantic versioningに従って比較します。

### ストリーム演算子

```cpp
std::ostream& operator<<(std::ostream& os, const semantic_version& ver);
```

`{major}.{minor}.{patch}`の形式で出力します。

### `to_string`

```cpp
std::string to_string(const semantic_version& ver);
```

`{major}.{minor}.{patch}`の形式で文字列化します。

# `toml::spec`

`spec`は、TOMLのバージョン情報を格納するクラスです。

```cpp
struct spec
{
    constexpr static spec default_version() noexcept;

    constexpr static spec v(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept;

    constexpr explicit spec(const semantic_version& semver) noexcept;

    semantic_version version; // toml version

    // diff from v1.0.0 -> v1.1.0
    bool v1_1_0_allow_control_characters_in_comments;
    bool v1_1_0_allow_newlines_in_inline_tables;
    bool v1_1_0_allow_trailing_comma_in_inline_tables;
    bool v1_1_0_allow_non_english_in_bare_keys;
    bool v1_1_0_add_escape_sequence_e;
    bool v1_1_0_add_escape_sequence_x;
    bool v1_1_0_make_seconds_optional;

    // library extensions
    bool ext_hex_float;  // allow hex float
    bool ext_num_suffix; // allow number suffix
    bool ext_null_value; // allow null value
};
```

## メンバ関数

### コンストラクタ

```cpp
constexpr explicit spec(const semantic_version& semver) noexcept;
```

指定されたTOMLバージョンで`spec`を構築します。

TOML v1.0.0と、TOML v1.1.0に対応しています。

### `default_version()`

```cpp
constexpr static spec default_version() noexcept;
```

デフォルトのバージョンで`spec`を構築します。

`toml::parse`、`toml::format`でのデフォルト値として使われます。

toml11 v4.4.0での値は、v1.0.0です。

### `v(major, minor, patch)`

```cpp
constexpr static spec v(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept;
```

指定されたバージョンで`spec`を構築します。

## メンバ変数

各フラグは機能追加がされたバージョンを指定されたとき、自動的に`true`になります。

変更して渡すことで、`toml::parse`、`toml::format`の挙動を変更できます。

{{<hint warning>}}

TOML v1.1.0の一部の機能にはかなり長い議論が続いており、まだ差し戻される可能性があります。

実際に差し戻された場合、toml11はマイナーバージョンアップでそれらの機能を削除、もしくは対応するそれ以降のバージョンに移動します。

そのような意味で、将来のバージョンに関する機能は全て不安定なものと考えてください。

{{</hint>}}

### 例

```cpp
auto spec = toml::spec::v(1, 0, 0);
// v1.0.0の機能に追加して、inline table内の改行を許可する。
// それ以外のv1.1.0の機能は有効化されない。
spec.v1_1_0_allow_newlines_in_inline_tables = true;

auto input = toml::parse("input_file.toml", spec);
```

### `v1_1_0_allow_control_characters_in_comments`

```cpp
bool v1_1_0_allow_control_characters_in_comments;
```

ほとんどの制御文字をコメントに含むことを許可します。

toml v1.1.0で追加。

### `v1_1_0_allow_newlines_in_inline_tables`

```cpp
bool v1_1_0_allow_newlines_in_inline_tables;
```

inline table内で改行することを許可します。

toml v1.1.0で追加。

### `v1_1_0_allow_trailing_comma_in_inline_tables`

```cpp
bool v1_1_0_allow_trailing_comma_in_inline_tables;
```

inline table内での末尾コンマを許可します。

toml v1.1.0で追加。

### `v1_1_0_add_escape_sequence_e`

```cpp
bool v1_1_0_add_escape_sequence_e;
```

`\e`でESC文字を指定できるようになります。

toml v1.1.0で追加。

### `v1_1_0_add_escape_sequence_x`

```cpp
bool v1_1_0_add_escape_sequence_x;
```

`\xHH`で1バイトの文字を指定できるようになります。

toml v1.1.0で追加。

### `v1_1_0_make_seconds_optional`

```cpp
bool v1_1_0_make_seconds_optional;
```

時刻での秒数指定を省略可能にします。

指定されなかった秒数は`0`で初期化されます。

toml v1.1.0で追加。

### `ext_hex_float`

```cpp
bool ext_hex_float;
```

toml11限定の言語拡張です。

どのバージョンを指定しても、`false`で初期化されます。
使用する際は個別に`true`にしてください。

浮動小数点数の16進数表記を許可します。

16進数表記は`printf`で`%a/%A`を指定した場合に準拠します。

```
hexf = 0xC0FFEEp-10
```

`toml::format` は、渡された `toml::spec` で `ext_hex_format` が `true` の場合のみ
16進表記でフォーマットします。
フォーマット情報で `hex` が指定されているにも関わらず `toml::format` に渡された
`toml::spec` の `ext_hex_float` が `false` だった場合、16進数指定は無視され、
10進表記で最大の精度で出力されます。

### `ext_num_suffix`

```cpp
bool ext_num_suffix;
```

toml11限定の言語拡張です。

どのバージョンを指定しても、`false`で初期化されます。
使用する際は個別に`true`にしてください。

10進数の整数と浮動小数点数に接尾辞を追加します。型を問わず、16進や8進、2進表記には適用されません。

数値と接尾辞の間は`_`で区切られている必要があります。

数値部分との区別のため、接尾辞は数値で始まることはできません。

```
distance = 10_m   # valid
distance = 10_2m  # invalid
distance = 10_2_m # valid
```

接尾辞は `std::string suffix` としてフォーマット情報に保持されます。
数値部分とを分ける `_` は `suffix` に含まれません。

```cpp
toml::value distance = toml::find(input, "distance");
assert(distance.as_integer_fmt().suffix == std::string("m"));
```

`toml::format` は、渡された `toml::spec` の `ext_num_suffix` が `true` の場合のみ
これをフォーマットします。

`suffix`は以下のような文法を持ちます。

```abnf
non-digit-graph = ALPHA / non-ascii
graph           = ALPHA / DIGIT / non-ascii
suffix          = _ non-digit-graph *( graph / ( _ graph ) )
```

### `ext_null_value`

```cpp
bool ext_null_value;
```

toml11限定の言語拡張です。

値として `null` を許可します。

`null` を指定された `toml::value` は値を持たず、 `is_empty()` が `true` になります。

`toml::format` は、渡された `toml::spec` で `ext_null_value` が `true` の場合のみ
`null` としてフォーマットします。
そうでない場合、 `toml::format` がエラーで終了します。

+++
title = "value.hpp"
type  = "docs"
+++

# value.hpp

`value.hpp`では、`basic_value`が定義されます。

# `toml::basic_value`

`basic_value`は、TOMLの値を格納するクラスです。

```cpp
namespace toml
{
template <class TypeConfig>
class basic_value;

// 以下はtypes.hppで定義される
// using value = basic_value<type_config>;
// using table = typename basic_value<type_config>::table_type;
// using array = typename basic_value<type_config>::array_type;

template<typename TC>
bool operator==(const basic_value<TC>&, const basic_value<TC>&);
template<typename TC>
bool operator!=(const basic_value<TC>&, const basic_value<TC>&);
template<typename TC>
bool operator< (const basic_value<TC>&, const basic_value<TC>&);
template<typename TC>
bool operator<=(const basic_value<TC>&, const basic_value<TC>&);
template<typename TC>
bool operator> (const basic_value<TC>&, const basic_value<TC>&);
template<typename TC>
bool operator>=(const basic_value<TC>&, const basic_value<TC>&);
} //toml
```

## メンバ型

以下のメンバ型が定義されます。

`TypeConfig`を使って、メンバ型を変更することができます。

参考： [types.hpp]({{<ref "types.md">}})

| 名前                   | 定義 |
|:-----------------------|:-------------------------------------|
| `char_type`            | `typename TypeConfig::char_type`     |
| `key_type`             | `typename TypeConfig::string_type`   |
| `value_type`           | `basic_value<TypeConfig>`            |
| `boolean_type`         | `typename TypeConfig::boolean_type`  |
| `integer_type`         | `typename TypeConfig::integer_type`  |
| `floating_type`        | `typename TypeConfig::floating_type` |
| `string_type`          | `typename TypeConfig::string_type`   |
| `local_time_type`      | `toml::local_time`                   |
| `local_date_type`      | `toml::local_date`                   |
| `local_datetime_type`  | `toml::local_datetime`               |
| `offset_datetime_type` | `toml::offset_datetime`              |
| `array_type`           | `typename TypeConfig::template array_type<value_type>`|
| `table_type`           | `typename TypeConfig::template table_type<key_type, value_type>`|
| `comment_type`         | `typename TypeConfig::comment_type`  |

## メンバ関数

### デフォルトコンストラクタ

```cpp
basic_value() noexcept
```

空の`toml::value`を構築します。

構築された`toml::value`は空になります。

### コピー・ムーブコンストラクタ

```cpp
basic_value(const basic_value& v)
basic_value(basic_value&& v)
```

値、フォーマット情報、コメント、ファイル領域の全ての情報をコピー・ムーブします。

### コピー・ムーブコンストラクタ（コメント指定）

```cpp
basic_value(basic_value v, std::vector<std::string> com)
```

コメントを上書きしながらコピー・ムーブします。

### 変換コンストラクタ

```cpp
template<typename TI>
basic_value(basic_value<TI> other)
template<typename TI>
basic_value(basic_value<TI> other, std::vector<std::string> com)
```

異なる`type_config`を持つ`basic_value`からコピー・ムーブします。

### コンストラクタ (boolean)

```cpp
basic_value(boolean_type x)
basic_value(boolean_type x, boolean_format_info fmt)
basic_value(boolean_type x, std::vector<std::string> com)
basic_value(boolean_type x, boolean_format_info fmt, std::vector<std::string> com)
```

`bool`と、そのフォーマット情報、コメントを受け取って構築します。

### コンストラクタ (integer)

```cpp
basic_value(integer_type x)
basic_value(integer_type x, integer_format_info fmt)
basic_value(integer_type x, std::vector<std::string> com)
basic_value(integer_type x, integer_format_info fmt, std::vector<std::string> com)
```

`integer`と、そのフォーマット情報、コメントを受け取って構築します。

### コンストラクタ(floating)

```cpp
template<typename T, /* T は std::is_floating_point<T> を満たす */>
basic_value(T x)
template<typename T, /* T は std::is_floating_point<T> を満たす */>
basic_value(T x, floating_format_info fmt)
template<typename T, /* T は std::is_floating_point<T> を満たす */>
basic_value(T x, std::vector<std::string> com)
template<typename T, /* T は std::is_floating_point<T> を満たす */>
basic_value(T x, floating_format_info fmt, std::vector<std::string> com)
```

`floating`と、そのフォーマット情報、コメントを受け取って構築します。

### コンストラクタ(string)

```cpp
basic_value(string_type x)
basic_value(string_type x, string_format_info fmt)
basic_value(string_type x, std::vector<std::string> com)
basic_value(string_type x, string_format_info fmt, std::vector<std::string> com)

basic_value(const string_type::value_type* x)
basic_value(const string_type::value_type* x, string_format_info fmt)
basic_value(const string_type::value_type* x, std::vector<std::string> com)
basic_value(const string_type::value_type* x, string_format_info fmt, std::vector<std::string> com)

// C++17以降
basic_value(string_view_type x)
basic_value(string_view_type x, string_format_info fmt)
basic_value(string_view_type x, std::vector<std::string> com)
basic_value(string_view_type x, string_format_info fmt, std::vector<std::string> com)
```

`string`と、そのフォーマット情報、コメントを受け取って構築します。

`string_view_type`は、`string_type`と同じ`value_type`と`traits_type`を持ちます。

### コンストラクタ(local_date)

```cpp
basic_value(local_date_type x)
basic_value(local_date_type x, local_date_format_info fmt)
basic_value(local_date_type x, std::vector<std::string> com)
basic_value(local_date_type x, local_date_format_info fmt, std::vector<std::string> com)
```

`local_date_type`と、そのフォーマット情報、コメントを受け取って構築します。

### コンストラクタ(local_time)

```cpp
basic_value(local_time_type x)
basic_value(local_time_type x, local_time_format_info fmt)
basic_value(local_time_type x, std::vector<std::string> com)
basic_value(local_time_type x, local_time_format_info fmt, std::vector<std::string> com)

template<typename Rep, typename Period>
basic_value(const std::chrono::duration<Rep, Period>& x)
template<typename Rep, typename Period>
basic_value(const std::chrono::duration<Rep, Period>& x, local_time_format_info fmt)
template<typename Rep, typename Period>
basic_value(const std::chrono::duration<Rep, Period>& x, std::vector<std::string> com)
template<typename Rep, typename Period>
basic_value(const std::chrono::duration<Rep, Period>& x, local_time_format_info fmt, std::vector<std::string> com)
```

`local_time_type`と、そのフォーマット情報、コメントを受け取って構築します。

`std::chrono::duration`は、`00:00:00`からの時間幅として構築します。

### コンストラクタ(local_datetime)

```cpp
basic_value(local_datetime_type x)
basic_value(local_datetime_type x, local_date_format_info fmt)
basic_value(local_datetime_type x, std::vector<std::string> com)
basic_value(local_datetime_type x, local_date_format_info fmt, std::vector<std::string> com)
```

`local_datetime_type`と、そのフォーマット情報、コメントを受け取って構築します。

### コンストラクタ(offset_datetime)


```cpp
basic_value(offset_datetime_type x)
basic_value(offset_datetime_type x, offset_date_format_info fmt)
basic_value(offset_datetime_type x, std::vector<std::string> com)
basic_value(offset_datetime_type x, offset_date_format_info fmt, std::vector<std::string> com)

basic_value(std::chrono::system_clock::time_point x)
basic_value(std::chrono::system_clock::time_point x, offset_date_format_info fmt)
basic_value(std::chrono::system_clock::time_point x, std::vector<std::string> com)
basic_value(std::chrono::system_clock::time_point x, offset_date_format_info fmt, std::vector<std::string> com)
```

`offset_datetime_type`と、そのフォーマット情報、コメントを受け取って構築します。

`std::chrono::system_clock::time_point`の場合、それが指す時点として構築します。

### コンストラクタ(array)

```cpp
basic_value(array_type x)
basic_value(array_type x, integer_format_info fmt)
basic_value(array_type x, std::vector<std::string> com)
basic_value(array_type x, integer_format_info fmt, std::vector<std::string> com)

template<typename T, /* T is array-like */>
basic_value(T x)
template<typename T, /* T is array-like */>
basic_value(T x, array_format_info fmt)
template<typename T, /* T is array-like */>
basic_value(T x, std::vector<std::string> com)
template<typename T, /* T is array-like */>
basic_value(T x, array_format_info fmt, std::vector<std::string> com)
```

`array`と、そのフォーマット情報、コメントを受け取って構築します。

`array-like`は、以下の条件を満たす型です。

- `T::iterator` を持つ。
- `T::value_type` を持つ。
- `T::key_type` を持た**ない**。
- `T::mapped_type` を持た**ない**。
- `std::string` では**ない**。
- `std::string_view` では**ない**。(C++17以降)

### コンストラクタ(table)

```cpp
basic_value(table_type x)
basic_value(table_type x, integer_format_info fmt)
basic_value(table_type x, std::vector<std::string> com)
basic_value(table_type x, integer_format_info fmt, std::vector<std::string> com)

template<typename T, /* T is table-like */>
basic_value(T x)
template<typename T, /* T is table-like */>
basic_value(T x, table_format_info fmt)
template<typename T, /* T is table-like */>
basic_value(T x, std::vector<std::string> com)
template<typename T, /* T is table-like */>
basic_value(T x, table_format_info fmt, std::vector<std::string> com)
```

`table`と、そのフォーマット情報、コメントを受け取って構築します。

`table-like`は、以下の条件を満たす型です。

- `T::iterator` を持つ。
- `T::value_type` を持つ。
- `T::key_type` を持つ。
- `T::mapped_type` を持つ。

### コンストラクタ(user-defined)

```cpp
template<typename T /* toml::into<T>が定義されていること */>
basic_value(const T& ud);

template<typename T /* toml::into<T>が定義されていること */>
basic_value(const T& ud, std::vector<std::string> com);

template<typename T /* into<T>は定義されておらず、T{}.into_toml()が存在すること */>
basic_value(const T& ud);

template<typename T /* into<T>は定義されておらず、T{}.into_toml()が存在すること */>
basic_value(const T& ud, std::vector<std::string> com);
```

`toml::into<T>` が定義されていた場合、 `toml::into<T>(ud)` の結果から構築します。

`toml::into<T>` が定義されておらず、 `T` に `into_toml()` メンバ関数が定義されていた場合、
`ud.into_toml()`の結果から構築します。

-----

### `operator=(basic_value)`

```cpp
basic_value& operator=(const basic_value& v)
basic_value& operator=(basic_value&& v)
template<typename TI>
basic_value& operator=(basic_value<TI> other)
```

右辺の`basic_value`を代入します。

### `operator=(T)`

```cpp
template<typename T>
basic_value& operator=(T x)
```

Tに対応する値を代入します。

`source_location`の指す内容は破棄されます。

もし同じ型の値を持っていたなら、元のフォーマット情報が保持されます。

-----

### `is<T>()`

```cpp
bool is<T>() const noexcept
```

####  条件

`T`は厳密にTOML型であること。つまり、値に対応する`toml::value::xxx_type`のいずれかであること。

#### 戻り値

格納している型が`T`と一致した場合`true`を、そうでない場合は`false`を返します。

-----

### `is(toml::value_t)`

```cpp
bool is(toml::value_t t) const noexcept
```

#### 戻り値

格納している型のタグが`t`と一致した場合`true`を、そうでない場合は`false`を返します。

-----

### `is_xxx()`

```cpp
bool is_boolean()         const noexcept;
bool is_integer()         const noexcept;
bool is_floating()        const noexcept;
bool is_string()          const noexcept;
bool is_offset_datetime() const noexcept;
bool is_local_datetime()  const noexcept;
bool is_local_date()      const noexcept;
bool is_local_time()      const noexcept;
bool is_array()           const noexcept;
bool is_table()           const noexcept;
```

#### 戻り値

格納している型がその型である場合`true`を、そうでない場合は`false`を返します。

-----

### `is_empty()`

```cpp
bool is_empty()   const noexcept;
```

#### 戻り値

デフォルト構築され値が代入されていない場合`true`を、そうでない場合は`false`を返します。

### `is_array_of_tables()`

```cpp
bool is_array_of_tables() const noexcept;
```

#### 戻り値

格納している型が配列であり、空ではなく、全要素がテーブルの場合は`true`を、そうでない場合は`false`を返します。

-----

### `type()`

```cpp
toml::value_t type() const noexcept
```

#### 戻り値

格納している型に対応するタグを返します。

-----

### `as_xxx()`

```cpp
boolean_type         const& as_boolean        () const;
integer_type         const& as_integer        () const;
floating_type        const& as_floating       () const;
string_type          const& as_string         () const;
offset_datetime_type const& as_offset_datetime() const;
local_datetime_type  const& as_local_datetime () const;
local_date_type      const& as_local_date     () const;
local_time_type      const& as_local_time     () const;
array_type           const& as_array          () const;
table_type           const& as_table          () const;

boolean_type        & as_boolean        ();
integer_type        & as_integer        ();
floating_type       & as_floating       ();
string_type         & as_string         ();
offset_datetime_type& as_offset_datetime();
local_datetime_type & as_local_datetime ();
local_date_type     & as_local_date     ();
local_time_type     & as_local_time     ();
array_type          & as_array          ();
table_type          & as_table          ();
```

#### 戻り値

指定された型への参照を返します。

#### 例外

格納されている値の型が指定と異なる場合、`toml::type_error`を送出します。

-----

### `as_xxx(std::nothrow)`

`std::nothrow`オブジェクトを渡して呼び出します。

```cpp
boolean_type         const& as_boolean        (const std::nothrow_t&) const noexcept;
integer_type         const& as_integer        (const std::nothrow_t&) const noexcept;
floating_type        const& as_floating       (const std::nothrow_t&) const noexcept;
string_type          const& as_string         (const std::nothrow_t&) const noexcept;
offset_datetime_type const& as_offset_datetime(const std::nothrow_t&) const noexcept;
local_datetime_type  const& as_local_datetime (const std::nothrow_t&) const noexcept;
local_date_type      const& as_local_date     (const std::nothrow_t&) const noexcept;
local_time_type      const& as_local_time     (const std::nothrow_t&) const noexcept;
array_type           const& as_array          (const std::nothrow_t&) const noexcept;
table_type           const& as_table          (const std::nothrow_t&) const noexcept;

boolean_type        & as_boolean        (const std::nothrow_t&) noexcept;
integer_type        & as_integer        (const std::nothrow_t&) noexcept;
floating_type       & as_floating       (const std::nothrow_t&) noexcept;
string_type         & as_string         (const std::nothrow_t&) noexcept;
offset_datetime_type& as_offset_datetime(const std::nothrow_t&) noexcept;
local_datetime_type & as_local_datetime (const std::nothrow_t&) noexcept;
local_date_type     & as_local_date     (const std::nothrow_t&) noexcept;
local_time_type     & as_local_time     (const std::nothrow_t&) noexcept;
array_type          & as_array          (const std::nothrow_t&) noexcept;
table_type          & as_table          (const std::nothrow_t&) noexcept;
```

#### 戻り値

指定された型への参照を返します。

#### 備考

格納されている値の型が指定と異なる場合、未定義動作となります。

-----

### `as_xxx_fmt()`

フォーマット情報にアクセスします。

```cpp
boolean_format_info        & as_boolean_fmt        ();
integer_format_info        & as_integer_fmt        ();
floating_format_info       & as_floating_fmt       ();
string_format_info         & as_string_fmt         ();
offset_datetime_format_info& as_offset_datetime_fmt();
local_datetime_format_info & as_local_datetime_fmt ();
local_date_format_info     & as_local_date_fmt     ();
local_time_format_info     & as_local_time_fmt     ();
array_format_info          & as_array_fmt          ();
table_format_info          & as_table_fmt          ();

boolean_format_info         const& as_boolean_fmt        () const;
integer_format_info         const& as_integer_fmt        () const;
floating_format_info        const& as_floating_fmt       () const;
string_format_info          const& as_string_fmt         () const;
offset_datetime_format_info const& as_offset_datetime_fmt() const;
local_datetime_format_info  const& as_local_datetime_fmt () const;
local_date_format_info      const& as_local_date_fmt     () const;
local_time_format_info      const& as_local_time_fmt     () const;
array_format_info           const& as_array_fmt          () const;
table_format_info           const& as_table_fmt          () const;
```

#### 戻り値

指定された型のフォーマット情報を持つ構造体への参照を返します。

#### 例外

格納されている値の型が指定と異なる場合、`toml::type_error`を送出します。

-----

### `as_xxx_fmt(std::nothrow)`

`std::nothrow`オブジェクトを渡して呼び出します。

```cpp
boolean_format_info        & as_boolean_fmt        (const std::nothrow_t&) noexcept;
integer_format_info        & as_integer_fmt        (const std::nothrow_t&) noexcept;
floating_format_info       & as_floating_fmt       (const std::nothrow_t&) noexcept;
string_format_info         & as_string_fmt         (const std::nothrow_t&) noexcept;
offset_datetime_format_info& as_offset_datetime_fmt(const std::nothrow_t&) noexcept;
local_datetime_format_info & as_local_datetime_fmt (const std::nothrow_t&) noexcept;
local_date_format_info     & as_local_date_fmt     (const std::nothrow_t&) noexcept;
local_time_format_info     & as_local_time_fmt     (const std::nothrow_t&) noexcept;
array_format_info          & as_array_fmt          (const std::nothrow_t&) noexcept;
table_format_info          & as_table_fmt          (const std::nothrow_t&) noexcept;

boolean_format_info         const& as_boolean_fmt        (const std::nothrow_t&) const noexcept;
integer_format_info         const& as_integer_fmt        (const std::nothrow_t&) const noexcept;
floating_format_info        const& as_floating_fmt       (const std::nothrow_t&) const noexcept;
string_format_info          const& as_string_fmt         (const std::nothrow_t&) const noexcept;
offset_datetime_format_info const& as_offset_datetime_fmt(const std::nothrow_t&) const noexcept;
local_datetime_format_info  const& as_local_datetime_fmt (const std::nothrow_t&) const noexcept;
local_date_format_info      const& as_local_date_fmt     (const std::nothrow_t&) const noexcept;
local_time_format_info      const& as_local_time_fmt     (const std::nothrow_t&) const noexcept;
array_format_info           const& as_array_fmt          (const std::nothrow_t&) const noexcept;
table_format_info           const& as_table_fmt          (const std::nothrow_t&) const noexcept;
```

#### 戻り値

指定された型のフォーマット情報を持つ構造体への参照を返します。

#### 備考

格納されている値の型が指定と異なる場合、未定義動作となります。

-----

### `at(key)`

```cpp
value_type&       at(const key_type& key);
value_type const& at(const key_type& key) const;
```

#### 戻り値

今の`value`を`table`にキャストしたあと、`key`によって指定される要素を返します。

#### 例外

もし格納している値が`table`ではなかった場合、`toml::type_error`を送出します。

もし格納している`table`が指定された要素を持っていなかった場合、`std::out_of_range`を送出します。

-----

#### `operator[](key)`

```cpp
value_type& operator[](const key_type& k);
```

##### 戻り値

今の`value`を`table`にキャストしたあと、`key`によって指定される要素への参照です。

もし`key`によって指定される要素が存在しない場合、デフォルト構築されます。

##### 例外

もし格納している値が`table`ではなかった場合、`toml::type_error`を送出します。

-----

### `count(key)`

```cpp
std::size_t count(const key_type& key) const;
```

#### 戻り値

今の`value`を`table`にキャストしたあと、`key`に対応する要素が含まれていれば`1`、そうでなければ`0`を返します。

#### 例外

もし格納している値が`table`ではなかった場合、`toml::type_error`を送出します。

-----

### `contains(key)`

```cpp
bool contains(const key_type& key) const;
```

#### 戻り値

今の`value`を`table`にキャストしたあと、`key`に対応する要素が含まれていれば`true`、そうでなければ`false`を返します。

#### 例外

もし格納している値が`table`ではなかった場合、`toml::type_error`を送出します。

-----

### `at(idx)`

```cpp
value_type&       at(const std::size_t idx);
value_type const& at(const std::size_t idx) const;
```

#### 戻り値

今の`value`を`array`にキャストしたあと、`idx`によって指定される要素を返します。

#### 例外

もし格納している値が`array`ではなかった場合、`toml::type_error`を送出します。

もし格納している`array`が指定された要素を持っていなかった場合、`std::out_of_range`を送出します。

-----

### `operator[](idx)`

```cpp
value_type&       operator[](const std::size_t idx)       noexcept;
value_type const& operator[](const std::size_t idx) const noexcept;
```

#### 戻り値

今の`value`を`array`にキャストしたあと、`idx`によって指定される要素への参照を返します。

#### 備考

一切のチェックを行いません。

もし格納している値が`array`ではなかった場合、あるいは`idx`によって指定される要素が存在しない場合、未定義動作となります。

-----

### `push_back(value)`

```cpp
void push_back(const value_type& x);
void push_back(value_type&& x);
```

`value`を`array`にキャストしたのち、その`array`に対して`push_back`を実行します。

#### 戻り値

なし。

#### 例外

格納している値が`array`ではなかった場合、`toml::type_error`を送出します。

-----

### `emplace_back(args...)`

```cpp
template<typename ... Ts>
value_type& emplace_back(Ts&& ... args)
```

`value`を`array`にキャストしたのち、その`array`に対して`emplace_back`を実行します。

#### 戻り値

構築した値への参照。

#### 例外

格納している値が`array`ではなかった場合、`toml::type_error`を送出します。

-----

### `size()`

```cpp
std::size_t size() const;
```

#### 戻り値

今の`value`を`array`、`string`、`table`のどれかにキャストしたあと、その要素数を返します。
`string`の場合、文字数を返します。

#### 例外

格納している値が`array`, `string`, `table`のどれでもなかった場合、`toml::type_error`を送出します。

-----

### `location()`

```cpp
source_location location() const;
```

#### 戻り値

その`value`が定義されたTOML文書内の位置を表す`source_location`オブジェクトを返します。

もしTOML文書のパースによって構築されたものでない場合、どこも指示さない`source_location`を返します。

-----

### `comments()`

```cpp
comment_type const& comments() const noexcept;
comment_type&       comments()       noexcept;
```

#### 戻り値

コメント用コンテナへの参照を返します。

-----

### `accessed()`

```cpp
bool accessed() const;
```

#### 戻り値

その`value`がパース後に一度でも`as_xxx`や`is_xxx`でアクセスされていた場合、`true`を返します。
それ以外の場合、`false`を返します。

#### 備考

`TOML11_ENABLE_ACCESS_CHECK`が定義されている場合にのみ存在します。

## 非メンバ関数

### `operator==`

```cpp
template<typename TC>
bool operator==(const basic_value<TC>&, const basic_value<TC>&);
```

以下を満たすとき、2つの`basic_value<T>`は同値となります。

- TOML型が同一
- 含む値が同一
- コメントがバイト単位で同一

### `operator!=`

```cpp
template<typename TC>
bool operator!=(const basic_value<TC>& lhs, const basic_value<TC>& rhs)
{
    return !(lhs == rhs);
}
```

### `operator<`

`array_type`と`table_type`が`operator<`を持っている場合のみ定義されます。

```cpp
template<typename TC>
bool operator<(const basic_value<TC>&, const basic_value<TC>&);
```

以下の順番で比較されます。

1. TOML型
2. TOML型が同一の場合、その値
3. TOML型とその値が同一の場合、コメント

TOML型は、以下の順に小さい値を持ちます。

1. `toml::value_t::empty`
2. `toml::value_t::boolean`
3. `toml::value_t::integer`
4. `toml::value_t::floating`
5. `toml::value_t::string`
6. `toml::value_t::offset_datetime`
7. `toml::value_t::local_datetime`
8. `toml::value_t::local_date`
9. `toml::value_t::local_time`
10. `toml::value_t::array`
11. `toml::value_t::table`

### `operator<=`

`array_type`と`table_type`が`operator<`を持っている場合のみ定義されます。

```cpp
template<typename TC>
bool operator<=(const basic_value<TC>& lhs, const basic_value<TC>& rhs)
{
    return (lhs < rhs) || (lhs == rhs);
}
```

### `operator>`

`array_type`と`table_type`が`operator<`を持っている場合のみ定義されます。

```cpp
template<typename TC>
bool operator>(const basic_value<TC>& lhs, const basic_value<TC>& rhs)
{
    return !(lhs <= rhs);
}
```

### `operator>=`

`array_type`と`table_type`が`operator<`を持っている場合のみ定義されます。

```cpp
template<typename TC>
bool operator>=(const basic_value<TC>& lhs, const basic_value<TC>& rhs)
{
    return !(lhs < rhs);
}
```

# `toml::type_error`

型エラーの際に送出される例外です。

型エラーが生じた値の位置情報が格納されています。

```cpp
struct type_error final : public ::toml::exception
{
  public:
    type_error(std::string what_arg, source_location loc);
    ~type_error() noexcept override = default;

    const char* what() const noexcept override;

    source_location const& location() const noexcept;
};
```

# `toml::make_error_info`

```cpp
template<typename TC, typename ... Ts>
error_info make_error_info(
    std::string title, const basic_value<TC>& v, std::string msg, Ts&& ... tail);
```

`basic_value` の `location()` を呼び出して、その `source_location` を
[`make_error_info`]({{<ref "docs/reference/error_info#make_error_info">}})
に渡して `error_info` を作成します。

詳しくは [`error_info`]({{<ref "docs/reference/error_info">}}) を参照してください。

# `toml::format_error`

```cpp
template<typename TC, typename ... Ts>
std::string format_error(std::string title,
                         const basic_value<TC>& v, std::string msg, Ts&& ... tail);
```

`basic_value` の `location()` を呼び出して、その `source_location` を
[`format_error`]({{<ref "docs/reference/error_info#format_error">}})
に渡して `error_info` を作成し、それを文字列化して返します。

詳しくは [`error_info`]({{<ref "docs/reference/error_info">}}) を参照してください。

# 関連項目

- [comments.hpp]({{<ref "comments.md">}})
- [source_location.hpp]({{<ref "source_location.md">}})
- [types.hpp]({{<ref "types.md">}})
- [visit.hpp]({{<ref "visit.md">}})

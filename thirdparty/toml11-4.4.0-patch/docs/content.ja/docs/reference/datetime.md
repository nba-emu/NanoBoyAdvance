+++
title = "datetime.hpp"
type  = "docs"
+++

# datetime.hpp

TOMLの`datetime`で使用される、日時情報を保存するクラスを定義します。

# `enum class month_t`

月を指定する`enum class`です。

`std::tm`との関係で、`local_date`は1月を`0`としています。
混乱を避けるため、月の名前で指定できるよう`month_t`が用意されています。

```cpp
namespace toml
{
enum class month_t : std::uint8_t
{
    Jan =  0,
    Feb =  1,
    Mar =  2,
    Apr =  3,
    May =  4,
    Jun =  5,
    Jul =  6,
    Aug =  7,
    Sep =  8,
    Oct =  9,
    Nov = 10,
    Dec = 11
};
}
```

# `local_date`

日付を保持する構造体です。

`year`は西暦を、`month`は`std::tm`との対応のため1月を`0`として、`day`は日付を保持します。

```cpp
namespace toml
{
struct local_date
{
    std::int16_t year;
    std::uint8_t month;
    std::uint8_t day;

    local_date() = default;
    ~local_date() = default;
    local_date(local_date const&) = default;
    local_date(local_date&&)      = default;
    local_date& operator=(local_date const&) = default;
    local_date& operator=(local_date&&)      = default;

    local_date(int y, month_t m, int d);
    explicit local_date(const std::tm& t);
    explicit local_date(const std::chrono::system_clock::time_point& tp);
    explicit local_date(const std::time_t t);

    operator std::chrono::system_clock::time_point() const;
    operator std::time_t() const;
};

bool operator==(const local_date&, const local_date&);
bool operator!=(const local_date&, const local_date&);
bool operator< (const local_date&, const local_date&);
bool operator<=(const local_date&, const local_date&);
bool operator> (const local_date&, const local_date&);
bool operator>=(const local_date&, const local_date&);

std::ostream& operator<<(std::ostream& os, const local_date& date);
std::string to_string(const local_date& date);
}
```

## メンバ変数

### `year`

```cpp
std::int16_t year;
```

西暦です。オフセットはありません。`2024`年は`2024`です。

### `month`

```cpp
std::uint8_t month;
```

月を表します。`std::tm`との対応のため、1月は`0`, 2月は`1`と続きます。

混乱を避けるため、構築の際は`month_t`を使用します。

### `day`

```cpp
std::uint8_t day;
```

日付を表します。1日は`1`です。

## メンバ関数

### コンストラクタ

```cpp
local_date() = default;
```

デフォルト実装を使用します。

### デストラクタ

```cpp
~local_date() = default;
```

デフォルト実装を使用します。

### コピー・ムーブコンストラクタ

```cpp
local_date(local_date const&) = default;
local_date(local_date&&)      = default;
```

デフォルト実装を使用します。

### コピー・ムーブ代入演算子

```cpp
local_date& operator=(local_date const&) = default;
local_date& operator=(local_date&&)      = default;
```

デフォルト実装を使用します。

### コンストラクタ(`int year, month_t month, int day`)

```cpp
local_date(int y, month_t m, int d);
```

指定した値から`local_date`を構築します。

境界チェックなどは行いません。

### コンストラクタ(`std::tm`)

```cpp
local_date(const std::tm&);
```

指定した値から`local_date`を構築します。

### コンストラクタ(`std::chrono::system_clock::time_point`)

```cpp
local_date(const std::chrono::system_clock::time_point&);
```

指定した値から`local_date`を構築します。

タイムゾーンは実行環境でのものが選択されます。

### コンストラクタ(`std::time_t`)

```cpp
local_date(const std::time_t);
```

指定した値から`local_date`を構築します。

タイムゾーンは実行環境でのものが選択されます。

### `operator std::chrono::system_clock::time_point`

```cpp
operator std::chrono::system_clock::time_point() const;
```

`std::chrono::system_clock::time_point`に変換します。

タイムゾーンは実行環境でのものが選択されます。

時刻は0時00分とします。

### `operator std::time_t`

```cpp
operator std::time_t() const;
```

`std::time_t`に変換します。

タイムゾーンは実行環境でのものが選択されます。

時刻は0時00分とします。

## 非メンバ関数

### 比較演算子

```cpp
bool operator==(const local_date&, const local_date&);
bool operator!=(const local_date&, const local_date&);
bool operator< (const local_date&, const local_date&);
bool operator<=(const local_date&, const local_date&);
bool operator> (const local_date&, const local_date&);
bool operator>=(const local_date&, const local_date&);
```

日付の順序によって比較します。

### ストリーム出力演算子

```cpp
std::ostream& operator<<(std::ostream& os, const local_date& date);
```

TOMLのデフォルトのフォーマットで出力を行います。

### `to_string`

```cpp
std::string to_string(const local_date& date);
```

TOMLのデフォルトのフォーマットで文字列化します。

# `local_time`

```cpp
namespace toml
{
struct local_time
{
    std::uint8_t  hour;        // [0, 23]
    std::uint8_t  minute;      // [0, 59]
    std::uint8_t  second;      // [0, 60]
    std::uint16_t millisecond; // [0, 999]
    std::uint16_t microsecond; // [0, 999]
    std::uint16_t nanosecond;  // [0, 999]

    local_time(int h, int m, int s, int ms = 0, int us = 0, int ns = 0);

    explicit local_time(const std::tm& t);

    template<typename Rep, typename Period>
    explicit local_time(const std::chrono::duration<Rep, Period>& t);

    operator std::chrono::nanoseconds() const;

    local_time() = default;
    ~local_time() = default;
    local_time(local_time const&) = default;
    local_time(local_time&&)      = default;
    local_time& operator=(local_time const&) = default;
    local_time& operator=(local_time&&)      = default;
};

bool operator==(const local_time& lhs, const local_time& rhs);
bool operator!=(const local_time& lhs, const local_time& rhs);
bool operator< (const local_time& lhs, const local_time& rhs);
bool operator<=(const local_time& lhs, const local_time& rhs);
bool operator> (const local_time& lhs, const local_time& rhs);
bool operator>=(const local_time& lhs, const local_time& rhs);

std::ostream& operator<<(std::ostream& os, const local_time& time);
std::string to_string(const local_time& time);
}
```

## メンバ変数

### `hour`

```cpp
std::uint8_t  hour;
```

時間を表します。`0`から`23`の値を取ります。

### `minute`

```cpp
std::uint8_t  minute;      // [0, 59]
```

分を表します。`0`から`59`の値を取ります。

### `second`

```cpp
std::uint8_t  second;      // [0, 60]
```

秒を表します。`0`から`60`の値を取ります。

### `millisecond`

```cpp
std::uint16_t millisecond; // [0, 999]
```

ミリ秒を表します。`0`から`999`の値を取ります。

### `microsecond`

```cpp
std::uint16_t microsecond; // [0, 999]
```

マイクロ秒を表します。`0`から`999`の値を取ります。

### `nanosecond`

```cpp
std::uint16_t nanosecond;  // [0, 999]
```

ナノ秒を表します。`0`から`999`の値を取ります。

## メンバ関数

### デフォルトコンストラクタ

```cpp
local_time() = default;
```

全ての値を`0`で初期化します。

### コンストラクタ(h, m, s, ms = 0, us = 0, ns = 0)

```cpp
local_time(int h, int m, int s, int ms = 0, int us = 0, int ns = 0);
```

指定された時刻を使って構築します。

境界チェックは行われません。

### コンストラクタ(`std::tm`)

```cpp
explicit local_time(const std::tm& t);
```

`std::tm`の`tm_hour`, `tm_min`, `tm_sec`を使って構築します。

サブセコンドは全て`0`で初期化されます。

### コンストラクタ(`std::chrono::duration`)

```cpp
template<typename Rep, typename Period>
explicit local_time(const std::chrono::duration<Rep, Period>& t);
```

`duration`をその日の0時からの時間として構築します。

### `operator std::chrono::nanoseconds`

```cpp
operator std::chrono::nanoseconds() const;
```

`std::chrono::nanoseconds`へ変換します。

## 非メンバ関数

### 比較演算子

```cpp
bool operator==(const local_time& lhs, const local_time& rhs);
bool operator!=(const local_time& lhs, const local_time& rhs);
bool operator< (const local_time& lhs, const local_time& rhs);
bool operator<=(const local_time& lhs, const local_time& rhs);
bool operator> (const local_time& lhs, const local_time& rhs);
bool operator>=(const local_time& lhs, const local_time& rhs);
```

時刻の値によって比較を行います。

### ストリーム演算子

```cpp
std::ostream& operator<<(std::ostream& os, const local_time& time);
```

TOMLのデフォルトのフォーマットで出力を行います。

### `to_string`

```cpp
std::string to_string(const local_time& time);
```

TOMLのデフォルトのフォーマットで文字列化します。

# `time_offset`

```cpp
namespace toml
{
struct time_offset
{
    std::int8_t hour{0};   // [-12, 12]
    std::int8_t minute{0}; // [-59, 59]

    time_offset(int h, int m);

    operator std::chrono::minutes() const;

    time_offset() = default;
    ~time_offset() = default;
    time_offset(time_offset const&) = default;
    time_offset(time_offset&&)      = default;
    time_offset& operator=(time_offset const&) = default;
    time_offset& operator=(time_offset&&)      = default;
};

bool operator==(const time_offset&, const time_offset&);
bool operator!=(const time_offset&, const time_offset&);
bool operator< (const time_offset&, const time_offset&);
bool operator<=(const time_offset&, const time_offset&);
bool operator> (const time_offset&, const time_offset&);
bool operator>=(const time_offset&, const time_offset&);

std::ostream& operator<<(std::ostream& os, const time_offset& offset);
std::string to_string(const time_offset& offset);
}
```

## メンバ変数

### `hour`

```cpp
std::int8_t hour{0};   // [-12, 12]
```

時間のオフセットです。-12から+12の範囲の値を取ります。

### `minute`

```cpp
std::int8_t minute{0}; // [-59, 59]
```

分のオフセットです。-59から+59の範囲の値を取ります。

## メンバ関数

### コンストラクタ

```cpp
time_offset(int h, int m);
```

時間と分を取って構築します。

境界チェックは行いません。

### `operator std::chrono::minutes`

```cpp
operator std::chrono::minutes() const;
```

`std::chrono::minutes`への変換を行います。

## 非メンバ関数

### 比較演算子

```cpp
bool operator==(const time_offset&, const time_offset&);
bool operator!=(const time_offset&, const time_offset&);
bool operator< (const time_offset&, const time_offset&);
bool operator<=(const time_offset&, const time_offset&);
bool operator> (const time_offset&, const time_offset&);
bool operator>=(const time_offset&, const time_offset&);
```

時刻の長さで比較します。

### ストリーム出力演算子

```cpp
std::ostream& operator<<(std::ostream& os, const time_offset&);
```

TOMLのデフォルトのフォーマットで出力を行います。

### `to_string`

```cpp
std::string to_string(const time_offset&);
```

TOMLのデフォルトのフォーマットで文字列化します。

# `local_datetime`

```cpp
namespace toml
{
struct local_datetime
{
    local_date date;
    local_time time;

    local_datetime(local_date d, local_time t);

    explicit local_datetime(const std::tm& t);
    explicit local_datetime(const std::chrono::system_clock::time_point& tp);
    explicit local_datetime(const std::time_t t);

    operator std::chrono::system_clock::time_point() const;
    operator std::time_t() const;

    local_datetime() = default;
    ~local_datetime() = default;
    local_datetime(local_datetime const&) = default;
    local_datetime(local_datetime&&)      = default;
    local_datetime& operator=(local_datetime const&) = default;
    local_datetime& operator=(local_datetime&&)      = default;
};

bool operator==(const local_datetime&, const local_datetime&);
bool operator!=(const local_datetime&, const local_datetime&);
bool operator< (const local_datetime&, const local_datetime&);
bool operator<=(const local_datetime&, const local_datetime&);
bool operator> (const local_datetime&, const local_datetime&);
bool operator>=(const local_datetime&, const local_datetime&);

std::ostream& operator<<(std::ostream& os, const local_datetime& dt);
std::string to_string(const local_datetime& dt);
}
```

## メンバ変数

### `local_date date`

```cpp
local_date date;
```

日付部分のデータを保持します。

### `local_time time`

```cpp
local_time time;
```

時刻部分のデータを保持します。

## メンバ関数

### デフォルトコンストラクタ

`date`, `time`の双方をデフォルト構築します。

### コンストラクタ(`local_date, local_time`)

指定された`date`と`time`で構築します。

### コンストラクタ(`std::tm`)

`std::tm`から構築します。

タイムゾーンは実行環境でのものが選択されます。

### コンストラクタ(`std::chrono::system_clock::time_point`)

`std::chrono::system_clock::time_point`から構築します。

タイムゾーンは実行環境でのものが選択されます。

### コンストラクタ(`std::time_t`)

`std::time_t`から構築します。

タイムゾーンは実行環境でのものが選択されます。

### `operator std::chrono::system_clock::time_point`

`std::chrono::system_clock::time_point`へ変換します。

### `operator std::time_t`

`std::time_t`へ変換します。

## 非メンバ関数

### 比較演算子

```cpp
bool operator==(const local_datetime&, const local_datetime&);
bool operator!=(const local_datetime&, const local_datetime&);
bool operator< (const local_datetime&, const local_datetime&);
bool operator<=(const local_datetime&, const local_datetime&);
bool operator> (const local_datetime&, const local_datetime&);
bool operator>=(const local_datetime&, const local_datetime&);
```

日付順で比較します。

### ストリーム出力演算子

```cpp
std::ostream& operator<<(std::ostream& os, const local_datetime&);
```

TOMLのデフォルトのフォーマットで出力を行います。

### `to_string`

```cpp
std::string to_string(const local_datetime&);
```

TOMLのデフォルトのフォーマットで文字列化します。

# `offset_datetime`

```cpp
namespace toml
{
struct offset_datetime
{
    local_date  date;
    local_time  time;
    time_offset offset;

    offset_datetime(local_date d, local_time t, time_offset o);
    offset_datetime(const local_datetime& dt, time_offset o);
    explicit offset_datetime(const local_datetime& ld);
    explicit offset_datetime(const std::chrono::system_clock::time_point& tp);
    explicit offset_datetime(const std::time_t& t);
    explicit offset_datetime(const std::tm& t);

    operator std::chrono::system_clock::time_point() const;
    operator std::time_t() const;

    offset_datetime() = default;
    ~offset_datetime() = default;
    offset_datetime(offset_datetime const&) = default;
    offset_datetime(offset_datetime&&)      = default;
    offset_datetime& operator=(offset_datetime const&) = default;
    offset_datetime& operator=(offset_datetime&&)      = default;
};

bool operator==(const offset_datetime&, const offset_datetime&);
bool operator!=(const offset_datetime&, const offset_datetime&);
bool operator< (const offset_datetime&, const offset_datetime&);
bool operator<=(const offset_datetime&, const offset_datetime&);
bool operator> (const offset_datetime&, const offset_datetime&);
bool operator>=(const offset_datetime&, const offset_datetime&);

std::ostream& operator<<(std::ostream& os, const offset_datetime& dt);
std::string to_string(const offset_datetime& dt);
}
```

## メンバ変数

### `date`

```cpp
local_date date;
```

日付部分のデータを保持します。

### `time`

```cpp
local_time time;
```

時刻部分のデータを保持します。

### `offset`

```cpp
time_offset offset;
```

オフセット部分のデータを保持します。

## メンバ関数

### デフォルトコンストラクタ

`date`, `time`, `offset`を全てデフォルト構築します。

### コンストラクタ(`local_date, local_time, time_offset`)

指定された`date`, `time`, `offset`で構築します。

### コンストラクタ(`local_datetime, time_offset`)

`local_datetime`と`offset`から構築します。

### コンストラクタ(`std::tm`)

`std::tm`から構築します。

タイムゾーンはUTC(00:00)になります。

### コンストラクタ(`std::chrono::system_clock::time_point`)

`std::chrono::system_clock::time_point`から構築します。

タイムゾーンはUTC(00:00)になります。

### コンストラクタ(`std::time_t`)

`std::time_t`から構築します。

タイムゾーンはUTC(00:00)になります。

### `operator std::chrono::system_clock::time_point`

`std::chrono::system_clock::time_point`へ変換します。

タイムゾーンはUTC(00:00)になります。

### `operator std::time_t`

`std::time_t`へ変換します。

タイムゾーンはUTC(00:00)になります。

## 非メンバ関数

### 比較演算子

```cpp
bool operator==(const offset_datetime&, const offset_datetime&);
bool operator!=(const offset_datetime&, const offset_datetime&);
bool operator< (const offset_datetime&, const offset_datetime&);
bool operator<=(const offset_datetime&, const offset_datetime&);
bool operator> (const offset_datetime&, const offset_datetime&);
bool operator>=(const offset_datetime&, const offset_datetime&);
```

日付順で比較します。

同じ日付の場合、タイムゾーン順で比較されます。

### ストリーム出力演算子

```cpp
std::ostream& operator<<(std::ostream& os, const offset_datetime&);
```

TOMLのデフォルトのフォーマットで出力を行います。

### `to_string`

```cpp
std::string to_string(const offset_datetime&);
```

TOMLのデフォルトのフォーマットで文字列化します。


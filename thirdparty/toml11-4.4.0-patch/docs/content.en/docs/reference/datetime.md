+++
title = "datetime.hpp"
type  = "docs"
+++

# datetime.hpp

Defines a class that stores date and time information used in TOML's `datetime`.

# `enum class month_t`

Enum class to specify months.

Due to its relationship with `std::tm`, `local_date` treats January as `0`.
To avoid confusion, `month_t` allows specification of months by their names.

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

`local_date` holds a date.

`year` represents the year in AD. For `month`, January is represented as `0` to align with `std::tm`. `day` holds the day of the month.

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

## Member Variables

### `year`

```cpp
std::int16_t year;
```

Represents the year in AD. There's no offset. `2024` is simply `2024`.

### `month`

```cpp
std::uint8_t month;
```

Represents the month. To align with `std::tm`, January is `0`, February is `1`, and so on.

To avoid confusion, use `month_t` during construction.

### `day`

```cpp
std::uint8_t day;
```

Represents the day of the month. The first day is `1`.

## Member Functions

### Constructor

```cpp
local_date() = default;
```

Uses the default implementation.

### Destructor

```cpp
~local_date() = default;
```

Uses the default implementation.

### Copy and Move Constructors

```cpp
local_date(local_date const&) = default;
local_date(local_date&&)      = default;
```

Uses the default implementations.

### Copy and Move Assignment Operators

```cpp
local_date& operator=(local_date const&) = default;
local_date& operator=(local_date&&)      = default;
```

Uses the default implementations.

### Constructor (`int year, month_t month, int day`)

```cpp
local_date(int y, month_t m, int d);
```

Constructs a `local_date` from the specified values.

Does not perform boundary checks.

### Constructor (`std::tm`)

```cpp
local_date(const std::tm&);
```

Constructs a `local_date` from the specified `std::tm` value.

### Constructor (`std::chrono::system_clock::time_point`)

```cpp
local_date(const std::chrono::system_clock::time_point&);
```

Constructs a `local_date` from the specified `std::chrono::system_clock::time_point` value.

Time zone is determined by the environment.

### Constructor (`std::time_t`)

```cpp
local_date(const std::time_t);
```

Constructs a `local_date` from the specified `std::time_t` value.

Time zone is determined by the environment.

### `operator std::chrono::system_clock::time_point`

```cpp
operator std::chrono::system_clock::time_point() const;
```

Converts to `std::chrono::system_clock::time_point`.

Time zone is determined by the environment.

Time is set to 0 hours and 0 minutes.

### `operator std::time_t`

```cpp
operator std::time_t() const;
```

Converts to `std::time_t`.

Time zone is determined by the execution environment.

Time is set to 0 hours and 0 minutes.

## Non-member Functions

### Comparison Operators

```cpp
bool operator==(const local_date&, const local_date&);
bool operator!=(const local_date&, const local_date&);
bool operator< (const local_date&, const local_date&);
bool operator<=(const local_date&, const local_date&);
bool operator> (const local_date&, const local_date&);
bool operator>=(const local_date&, const local_date&);
```

Compares two dates.

### Stream Operators

```cpp
std::ostream& operator<<(std::ostream& os, const local_date& date);
```

Outputs in the default TOML format.

### `to_string`

```cpp
std::string to_string(const local_date& date);
```

Converts to a string in the default TOML format.

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

## Member Values

### `hour`

```cpp
std::uint8_t  hour;
```

Represents the hour. Values range from `0` to `23`.

### `minute`

```cpp
std::uint8_t  minute;      // [0, 59]
```

Represents the minute. Values range from `0` to `59`.

### `second`

```cpp
std::uint8_t  second;      // [0, 60]
```

Represents the second. Values range from `0` to `60`.

### `millisecond`

```cpp
std::uint16_t millisecond; // [0, 999]
```

Represents the millisecond. Values range from `0` to `999`.

### `microsecond`

```cpp
std::uint16_t microsecond; // [0, 999]
```

Represents the microsecond. Values range from `0` to `999`.

### `nanosecond`

```cpp
std::uint16_t nanosecond;  // [0, 999]
```

Represents the nanosecond. Values range from `0` to `999`.

## Member Functions

### default constructor

```cpp
local_time() = default;
```

Initializes all values to `0`.

### constructor (h, m, s, ms = 0, us = 0, ns = 0)

```cpp
local_time(int h, int m, int s, int ms = 0, int us = 0, int ns = 0);
```

Constructs using the specified time components.

No boundary checks are performed.

### constructor(`std::tm`)

```cpp
explicit local_time(const std::tm& t);
```

Constructs using `tm_hour`, `tm_min`, and `tm_sec` from `std::tm`.

Subseconds are initialized to `0`.

### constructor(`std::chrono::duration`)

```cpp
template<typename Rep, typename Period>
explicit local_time(const std::chrono::duration<Rep, Period>& t);
```

Constructs as the time of day from `0` hours of the day specified by `duration`.

### `operator std::chrono::nanoseconds`

```cpp
operator std::chrono::nanoseconds() const;
```

Converts to `std::chrono::nanoseconds`.

## Non-member Functions

### Comparison Operators

```cpp
bool operator==(const local_time& lhs, const local_time& rhs);
bool operator!=(const local_time& lhs, const local_time& rhs);
bool operator< (const local_time& lhs, const local_time& rhs);
bool operator<=(const local_time& lhs, const local_time& rhs);
bool operator> (const local_time& lhs, const local_time& rhs);
bool operator>=(const local_time& lhs, const local_time& rhs);
```

Compares based on time values.

### Stream Operator

```cpp
std::ostream& operator<<(std::ostream& os, const local_time& time);
```

Outputs in the default TOML format.

### `to_string`

```cpp
std::string to_string(const local_time& time);
```

Converts to a string in the default TOML format.

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

## Member Variables

### `hour`

```cpp
std::int8_t hour{0};   // [-12, 12]
```

Represents the hour offset, ranging from -12 to +12.

### `minute`

```cpp
std::int8_t minute{0}; // [-59, 59]
```

Represents the minute offset, ranging from -59 to +59.

## Member Functions

### Constructor

```cpp
time_offset(int h, int m);
```

Constructs with given hours and minutes.

No boundary checking is performed.

### `operator std::chrono::minutes`

```cpp
operator std::chrono::minutes() const;
```

Converts to `std::chrono::minutes`.

## Non-member Functions

### Comparison Operators

```cpp
bool operator==(const time_offset&, const time_offset&);
bool operator!=(const time_offset&, const time_offset&);
bool operator< (const time_offset&, const time_offset&);
bool operator<=(const time_offset&, const time_offset&);
bool operator> (const time_offset&, const time_offset&);
bool operator>=(const time_offset&, const time_offset&);
```

Compares based on time length.

### Stream Output Operator

```cpp
std::ostream& operator<<(std::ostream& os, const time_offset&);
```

Outputs in the default TOML format.

### `to_string`

```cpp
std::string to_string(const time_offset&);
```

Converts to a string in the default TOML format.


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

## Member Variables

### `local_date date`

```cpp
local_date date;
```

Stores the date component data.

### `local_time time`

```cpp
local_time time;
```

Stores the time component data.

## Member Functions

### Default Constructor

Constructs both `date` and `time` with default values.

### Constructor (`local_date, local_time`)

Constructs with the specified `date` and `time`.

### Constructor (`std::tm`)

Constructs from `std::tm`.

The timezone is selected based on the execution environment.

### Constructor (`std::chrono::system_clock::time_point`)

Constructs from `std::chrono::system_clock::time_point`.

The timezone is selected based on the execution environment.

### Constructor (`std::time_t`)

Constructs from `std::time_t`.

The timezone is selected based on the execution environment.

### `operator std::chrono::system_clock::time_point`

Converts to `std::chrono::system_clock::time_point`.

### `operator std::time_t`

Converts to `std::time_t`.

## Non-member Functions

### Comparison Operators

```cpp
bool operator==(const local_datetime&, const local_datetime&);
bool operator!=(const local_datetime&, const local_datetime&);
bool operator< (const local_datetime&, const local_datetime&);
bool operator<=(const local_datetime&, const local_datetime&);
bool operator> (const local_datetime&, const local_datetime&);
bool operator>=(const local_datetime&, const local_datetime&);
```

Compares based on chronological order.

### Stream Output Operator

```cpp
std::ostream& operator<<(std::ostream& os, const local_datetime&);
```

Outputs in the default TOML format.

### `to_string`

```cpp
std::string to_string(const local_datetime&);
```

Converts to a string in the default TOML format.

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

## Member Variables

### `date`

```cpp
local_date date;
```

Stores the date component.

### `time`

```cpp
local_time time;
```

Stores the time component.

### `offset`

```cpp
time_offset offset;
```

Stores the offset component.

## Member Functions

### Default Constructor

Constructs `date`, `time`, and `offset` with default values.

### Constructor (`local_date, local_time, time_offset`)

Constructs with the specified `date`, `time`, and `offset`.

### Constructor (`local_datetime, time_offset`)

Constructs from `local_datetime` and `offset`.

### Constructor (`std::tm`)

Constructs from `std::tm`.

The timezone is UTC (00:00).

### Constructor (`std::chrono::system_clock::time_point`)

Constructs from `std::chrono::system_clock::time_point`.

The timezone is UTC (00:00).

### Constructor (`std::time_t`)

Constructs from `std::time_t`.

The timezone is UTC (00:00).

### `operator std::chrono::system_clock::time_point`

Converts to `std::chrono::system_clock::time_point`.

The timezone is UTC (00:00).

### `operator std::time_t`

Converts to `std::time_t`.

The timezone is UTC (00:00).

## Non-member Functions

### Comparison Operators

```cpp
bool operator==(const offset_datetime&, const offset_datetime&);
bool operator!=(const offset_datetime&, const offset_datetime&);
bool operator< (const offset_datetime&, const offset_datetime&);
bool operator<=(const offset_datetime&, const offset_datetime&);
bool operator> (const offset_datetime&, const offset_datetime&);
bool operator>=(const offset_datetime&, const offset_datetime&);
```

Compares based on chronological order.

If dates are the same, compares based on timezone order.

### Stream Output Operator

```cpp
std::ostream& operator<<(std::ostream& os, const offset_datetime&);
```

Outputs in the default TOML format.

### `to_string`

```cpp
std::string to_string(const offset_datetime&);
```

Converts to a string in the default TOML format.

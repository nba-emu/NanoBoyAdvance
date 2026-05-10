+++
title = "value.hpp"
type  = "docs"
+++

# value.hpp

`value.hpp` defines `basic_value`.

# `toml::basic_value`

`basic_value` is a class that stores TOML values.

```cpp
namespace toml
{
template <class TypeConfig>
class basic_value;

// Defined in types.hpp
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

## Member Types

The following member types are defined.

You can modify the member types using `TypeConfig`.

See also: [types.hpp]({{<ref "types.md">}})

| Name                   | Definition |
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

## Member Functions

### Default Constructor

```cpp
basic_value() noexcept
```

Constructs an empty `toml::value`.

The constructed `toml::value` will be empty.

### Copy and Move Constructors

```cpp
basic_value(const basic_value& v)
basic_value(basic_value&& v)
```

Copies or moves all information including values, format information, comments, and file regions.

### Copy and Move Constructors with Comments

```cpp
basic_value(basic_value v, std::vector<std::string> com)
```

Copies or moves the object while overwriting comments.

### Conversion Constructors

```cpp
template<typename TI>
basic_value(basic_value<TI> other)
template<typename TI>
basic_value(basic_value<TI> other, std::vector<std::string> com)
```

Copies or moves from a `basic_value` with a different `type_config`.

### Constructor (boolean)

```cpp
basic_value(boolean_type x)
basic_value(boolean_type x, boolean_format_info fmt)
basic_value(boolean_type x, std::vector<std::string> com)
basic_value(boolean_type x, boolean_format_info fmt, std::vector<std::string> com)
```

Constructs an object with a `bool`, its format information, and comments.

### Constructor (integer)

```cpp
template<typename T, /* std::is_integral<T> is true */>
basic_value(T x)
template<typename T, /* std::is_integral<T> is true */>
basic_value(T x, integer_format_info fmt)
template<typename T, /* std::is_integral<T> is true */>
basic_value(T x, std::vector<std::string> com)
template<typename T, /* std::is_integral<T> is true */>
basic_value(T x, integer_format_info fmt, std::vector<std::string> com)
```

Constructs an object with an `integer`, its format information, and comments.

### Constructor (floating)

```cpp
template<typename T, /* std::is_floating_point<T> is true */>
basic_value(T x)
template<typename T, /* std::is_floating_point<T> is true */>
basic_value(T x, floating_format_info fmt)
template<typename T, /* std::is_floating_point<T> is true */>
basic_value(T x, std::vector<std::string> com)
template<typename T, /* std::is_floating_point<T> is true */>
basic_value(T x, floating_format_info fmt, std::vector<std::string> com)
```

Constructs an object with a `floating` point number, its format information, and comments.

### Constructor (string)

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

Constructs an object with a `string`, its format information, and comments.

`string_view_type` shares the same `value_type` and `traits_type` as `string_type`.

### Constructor (local_date)

```cpp
basic_value(local_date_type x)
basic_value(local_date_type x, local_date_format_info fmt)
basic_value(local_date_type x, std::vector<std::string> com)
basic_value(local_date_type x, local_date_format_info fmt, std::vector<std::string> com)
```

Constructs an object with a `local_date_type`, its format information, and comments.

### Constructor (local_time)

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

Constructs an object with a `local_time_type`, its format information, and comments.

For `std::chrono::duration`, constructs as a time span from `00:00:00`.

### Constructor (local_datetime)

```cpp
basic_value(local_datetime_type x)
basic_value(local_datetime_type x, local_date_format_info fmt)
basic_value(local_datetime_type x, std::vector<std::string> com)
basic_value(local_datetime_type x, local_date_format_info fmt, std::vector<std::string> com)
```

Constructs an object with a `local_datetime_type`, its format information, and comments.

### Constructor (offset_datetime)

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

Constructs an object with an `offset_datetime_type`, its format information, and comments.

For `std::chrono::system_clock::time_point`, constructs for the pointed time.

### Constructor (array)

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

Constructs an object with an `array`, its format information, and comments.

`array-like` types must meet the following criteria:

- Has `T::iterator`.
- Has `T::value_type`.
- Does **not** have `T::key_type`.
- Does **not** have `T::mapped_type`.
- Is **not** `std::string`.
- Is **not** `std::string_view` (since C++17).

### Constructor (table)

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

Constructs an object with a `table`, its format information, and comments.

`table-like` types must meet the following criteria:

- Has `T::iterator`.
- Has `T::value_type`.
- Has `T::key_type`.
- Has `T::mapped_type`.

### Constructor (user-defined)

```cpp
template<typename T /* toml::into<T> is defined */>
basic_value(const T& ud);

template<typename T /* toml::into<T> is defined */>
basic_value(const T& ud, std::vector<std::string> com);

template<typename T /* toml::into<T> is not defined, but T{}.into_toml() exists */>
basic_value(const T& ud);

template<typename T /* toml::into<T> is not defined, but T{}.into_toml() exists */>
basic_value(const T& ud, std::vector<std::string> com);
```

If `toml::into<T>` is defined, constructs from the result of `toml::into<T>(ud)`.

If `toml::into<T>` is not defined but `T` has a `into_toml()` member function, constructs from the result of `ud.into_toml()`.

-----

### `operator=(basic_value)`

```cpp
basic_value& operator=(const basic_value& v)
basic_value& operator=(basic_value&& v)
template<typename TI>
basic_value& operator=(basic_value<TI> other)
```

Assigns the right-hand side `basic_value` to the current object.

### `operator=(T)`

```cpp
template<typename T>
basic_value& operator=(T x)
```

Assigns a value corresponding to `T`.

The contents pointed to by `source_location` are discarded.

If the object already holds a value of the same type, the original format information is retained.

-----

### `is<T>()`

```cpp
bool is<T>() const noexcept
```

#### Requirements

`T` must be an exact TOML type, meaning it corresponds to one of the `toml::value::xxx_type`.

#### Return Value

Returns `true` if the stored type matches `T`, otherwise returns `false`.

-----

### `is(toml::value_t)`

```cpp
bool is(toml::value_t t) const noexcept
```

#### Return Value

Returns `true` if the tag of the stored type matches `t`, otherwise returns `false`.

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

#### Return Value

Returns `true` if the stored type matches the corresponding type, otherwise returns `false`.

------

### `is_empty()`

```cpp
bool is_empty()   const noexcept;
```

#### Return Value

Returns `true` if the object is default constructed and no value is assigned, otherwise returns `false`.

### `is_array_of_tables()`

```cpp
bool is_array_of_tables() const noexcept;
```

#### Return Value

Returns `true` if the stored type is an array that is not empty and all elements are tables, otherwise returns `false`.

-----

### `type()`

```cpp
toml::value_t type() const noexcept
```

#### Return Value

Returns the tag corresponding to the stored type.

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

#### Return Value

Returns a reference to the value of the specified type.

#### Exception

Throws `toml::type_error` if the stored value's type does not match the specified type.

-----

### `as_xxx(std::nothrow)`

Invoke with a `std::nothrow` object.

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

#### Return Value

Returns a reference to the value of the specified type.

#### Note

If the type of the stored value does not match the specified type, the behavior is undefined.

-----

### `as_xxx_fmt()`

Accesses format information.

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

#### Return Value

Returns a reference to the structure holding the format information for the specified type.

#### Exception

Throws `toml::type_error` if the stored value's type does not match the specified type.

-----

### `as_xxx_fmt(std::nothrow)`

Invoke with a `std::nothrow` object.

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

#### Return Value

Returns a reference to the structure holding the format information for the specified type.

#### Note

If the type of the stored value does not match the specified type, the behavior is undefined.

-----

### `at(key)`

```cpp
value_type&       at(const key_type& key);
value_type const& at(const key_type& key) const;
```

#### Return Value

Casts the current `value` to a `table` and returns the element specified by `key`.

#### Exception

Throws `toml::type_error` if the stored value is not a `table`.

Throws `std::out_of_range` if the `table` does not contain the specified element.

-----

#### `operator[](key)`

```cpp
value_type& operator[](const key_type& k);
```

##### Return Value

Casts the current `value` to a `table` and returns a reference to the element specified by `key`.

If the element specified by `key` does not exist, it is default-constructed.

##### Exception

Throws `toml::type_error` if the stored value is not a `table`.

-----

### `count(key)`

```cpp
std::size_t count(const key_type& key) const;
```

#### Return Value

Casts the current `value` to a `table` and returns `1` if the element corresponding to `key` is present, otherwise returns `0`.

#### Exception

Throws `toml::type_error` if the stored value is not a `table`.

-----

### `contains(key)`

```cpp
bool contains(const key_type& key) const;
```

#### Return Value

Casts the current `value` to a `table` and returns `true` if the element corresponding to `key` is present, otherwise returns `false`.

#### Exception

Throws `toml::type_error` if the stored value is not a `table`.

-----

### `at(idx)`

```cpp
value_type&       at(const std::size_t idx);
value_type const& at(const std::size_t idx) const;
```

#### Return Value

Casts the current `value` to an `array` and returns the element specified by `idx`.

#### Exception

Throws `toml::type_error` if the stored value is not an `array`.

Throws `std::out_of_range` if the specified element does not exist in the `array`.


-----

### `operator[](idx)`

```cpp
value_type&       operator[](const std::size_t idx)       noexcept;
value_type const& operator[](const std::size_t idx) const noexcept;
```

#### Return Value

Casts the current `value` to an `array` and returns a reference to the element specified by `idx`.

#### Note

Performs no checks. Behavior is undefined if the stored value is not an `array` or if the specified element does not exist.

-----

### `push_back(value)`

```cpp
void push_back(const value_type& x);
void push_back(value_type&& x);
```

Casts the current `value` to an `array` and performs `push_back` on the `array`.

#### Return Value

None.

#### Exception

Throws `toml::type_error` if the stored value is not an `array`.

-----

### `emplace_back(args...)`

```cpp
template<typename ... Ts>
value_type& emplace_back(Ts&& ... args)
```

Casts the current `value` to an `array` and performs `emplace_back` on the `array`.

#### Return Value

A reference to the constructed value.

#### Exception

Throws `toml::type_error` if the stored value is not an `array`.

-----

### `size()`

```cpp
std::size_t size() const;
```

#### Return Value

Casts the current `value` to an `array`, `string`, or `table` and returns the number of elements. For a `string`, it returns the number of characters.

#### Exception

Throws `toml::type_error` if the stored value is not an `array`, `string`, or `table`.

-----

### `location()`

```cpp
source_location location() const;
```

#### Return Value

Returns a `source_location` object representing the position within the TOML document where the `value` is defined.

If the `value` was not constructed by parsing a TOML document, returns a `source_location` that points to nowhere.

-----

### `comments()`

```cpp
comment_type const& comments() const noexcept;
comment_type&       comments()       noexcept;
```

#### Return Value

Returns a reference to the comment container.

-----

### `accessed()`

```cpp
bool accessed() const;
```

#### Return Value

Returns `true` only if the `value` has been accessed via `as_xxx` or `is_xxx`.
Otherwise, it returns `false`.

#### Remarks

It exists only when `TOML11_ENABLE_ACCESS_CHECK` is defined.

## Non-Member Functions

### `operator==`

```cpp
template<typename TC>
bool operator==(const basic_value<TC>&, const basic_value<TC>&);
```

Two `basic_value<T>` instances are considered equal if they satisfy the following conditions:

- The contained type is the same.
- The contained values are identical.
- The comments are identical at the byte level.

### `operator!=`

```cpp
template<typename TC>
bool operator!=(const basic_value<TC>& lhs, const basic_value<TC>& rhs)
{
    return !(lhs == rhs);
}
```

### `operator<`

Defined only if `array_type` and `table_type` have `operator<`.

```cpp
template<typename TC>
bool operator<(const basic_value<TC>&, const basic_value<TC>&);
```

Comparison order:

1. TOML type
2. If TOML types are the same, their values
3. If both the TOML types and values are the same, the comments

TOML types have the following order (from smallest to largest):

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

Defined only if `array_type` and `table_type` have `operator<`.

```cpp
template<typename TC>
bool operator<=(const basic_value<TC>& lhs, const basic_value<TC>& rhs)
{
    return (lhs < rhs) || (lhs == rhs);
}
```

### `operator>`

Defined only if `array_type` and `table_type` have `operator<`.

```cpp
template<typename TC>
bool operator>(const basic_value<TC>& lhs, const basic_value<TC>& rhs)
{
    return !(lhs <= rhs);
}
```

### `operator>=`

Defined only if `array_type` and `table_type` have `operator<`.

```cpp
template<typename TC>
bool operator>=(const basic_value<TC>& lhs, const basic_value<TC>& rhs)
{
    return !(lhs < rhs);
}
```

# `toml::type_error`

Exception thrown in case of a type error.

Contains the location information of the value that caused the type error.

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

Calls `location()` on a `basic_value`, passes the resulting `source_location` to
[`make_error_info`]({{<ref "docs/reference/error_info#make_error_info">}})
to create an `error_info`.

Refer to [`error_info`]({{<ref "docs/reference/error_info">}}) for more details.

# `toml::format_error`

```cpp
template<typename TC, typename ... Ts>
std::string format_error(std::string title,
                         const basic_value<TC>& v, std::string msg, Ts&& ... tail);
```

Calls `location()` on a `basic_value`, passes the resulting `source_location` to
[`format_error`]({{<ref "docs/reference/error_info#format_error">}})
to create an `error_info`, then converts it to a string and returns it.

Refer to [`error_info`]({{<ref "docs/reference/error_info">}}) for more details.

# Related

- [comments.hpp]({{<ref "comments.md">}})
- [source_location.hpp]({{<ref "source_location.md">}})
- [types.hpp]({{<ref "types.md">}})
- [visit.hpp]({{<ref "visit.md">}})

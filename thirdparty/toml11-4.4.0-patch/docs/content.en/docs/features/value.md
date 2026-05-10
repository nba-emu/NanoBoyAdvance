+++
title = "getting values"
type  = "docs"
weight = 20
+++

# Retrieving Values

This section explains how to access the values stored in `toml::value`.

## Accessing Values Using Member Functions

### `is_something` and `as_something`

`toml::value` has member functions like `is_boolean()` and `is_integer()` which allow you to check the type of the stored value.

Additionally, it has member functions like `as_boolean()` and `as_integer()` that allow you to access the value of that type.

For a complete list, refer to the [`toml::value` reference]({{<ref "docs/reference/value#is_xxx">}}).

```cpp
toml::value v = /* ... */;
if(v.is_string())
{
    std::cout << v.as_string() << std::endl;
}
```

If the stored value is of a different type than specified, a [`toml::type_error`]({{<ref "docs/reference/value#tomltype_error">}}) is thrown.

The `what()` method will contain a message like the following:

```
[error] toml::value::as_string(): bad_cast to string
 --> input.toml
   |
 1 | a = 123_456
   |     ^^^^^^^-- the actual type is integer
```

### `toml::value_t`

Type information can be identified using [`enum class toml::value_t`]({{<ref "docs/reference/value_t">}}).

The [`type()`]({{<ref "docs/reference/value#type">}}) member function returns the type information of the currently stored value.

```cpp
toml::value v = /* ... */;
switch(v.type())
{
    case toml:value_t::empty          : { /*...*/ break; }
    case toml:value_t::boolean        : { /*...*/ break; }
    case toml:value_t::integer        : { /*...*/ break; }
    case toml:value_t::floating       : { /*...*/ break; }
    case toml:value_t::string         : { /*...*/ break; }
    case toml:value_t::offset_datetime: { /*...*/ break; }
    case toml:value_t::local_datetime : { /*...*/ break; }
    case toml:value_t::local_date     : { /*...*/ break; }
    case toml:value_t::local_time     : { /*...*/ break; }
    case toml:value_t::array          : { /*...*/ break; }
    case toml:value_t::table          : { /*...*/ break; }
    default: {break;}
}
```

The [`is(toml::value_t)`]({{<ref "docs/reference/value#istomlvalue_t">}}) member function returns `true` if the stored value is of the given `value_t` type, otherwise it returns `false`.

```cpp
toml::value v = /* ... */;
if(v.is(toml::value_t::integer))
{
    std::cout << v.as_integer() << std::endl;
}
```

### `at`, `[]`, `contains`, `size`, `push_back`, `emplace_back`

`toml::value` provides some member functions similar to those of standard library containers.

These functions internally convert `toml::value` to the corresponding type and call the respective member functions.

#### `at(std::size_t i)`, `operator[](std::size_t i)`

These are equivalent to `as_array().at(i)` and `as_array()[i]`.

`toml::value` uses `std::vector<toml::value>` as `array_type` by default, so `at` throws `std::out_of_range` on error, while `operator[]` results in undefined behavior.

```cpp
toml::value v(toml::array{1,2,3});
std::cout << v.at(1);
```

If the stored type is not `array_type`, a `type_error` is thrown.

#### `at(std::string key)`, `operator[](std::string key)`

These are equivalent to `as_table().at(key)` and `as_table()[key]`.

`toml::value` uses `std::unordered_map<std::string, toml::value>` as `table_type` by default, so if the corresponding value does not exist, `at` throws `std::out_of_range`, while `operator[]` constructs a new `toml::value` and returns a reference to it. Therefore, there is no `const` version of `operator[]`.

```cpp
toml::value v(toml::table{});
v["a"] = 42;
```

If the stored type is not `table_type`, a `type_error` is thrown.

#### `size()`

Returns the length.

For `array_type` or `table_type`, it returns the number of elements; for `string_type`, it returns the number of characters (in bytes).

If the stored type is none of these, a `type_error` is thrown.

#### `push_back()`, `emplace_back()`

These are equivalent to `as_array().push_back()` and `as_array().emplace_back()`.

If the stored type is not `array_type`, a `type_error` is thrown.

## Accessing Comments

In toml11, comments are parsed by default and saved line by line with the corresponding value.

A comment corresponds to the value that comes immediately after the consecutive lines of comments, or to the value on the same line as the comment.

If there is no value immediately before or after the comment, it is not associated with any value and is ignored.

```toml
# input.toml

# This is a comment about a.
a = 42
b = 3.14 # This is a comment about b.

# This comment is ignored because it has no corresponding value.

# This is the 1st comment about c.
# This is the 2nd comment about c.
c = "foo" # This is the final comment about c.
# This comment is NOT a comment about c.
```

Comments corresponding to a value can be accessed using the `comments()` member function of `toml::value`.

`comments()` returns a container that has the same member functions as `std::vector<std::string>`.

```cpp
const auto v = toml::parse("input.toml");
const auto& a = v.at("a");
const auto& b = v.at("b");
const auto& c = v.at("c");

assert(a.comments().size() == 1);
assert(a.comments().at(0) == "# This is a comment about a.");

assert(b.comments().size() == 1);
assert(b.comments().at(0) == "# This is a comment about b.");

assert(c.comments().size() == 3);
assert(c.comments().at(0) == "# This is the 1st comment about c.");
assert(c.comments().at(1) == "# This is the 2nd comment about c.");
assert(c.comments().at(2) == "# This is the final comment about c.");
```

Comments related to the root table of the entire file are written at the beginning of the file.

```toml
# This is a comment about the root table.
# This is also a comment about the root table.

# This comment is ignored.

# This is a comment about a.
a = 42
```

However, if a value immediately follows the initial comment, the comment is interpreted as pertaining to that value, and there are no comments for the root table.

```toml
# This is a comment about a.
# This is also a comment about a.
a = 42
```

## Handling Inline Tables and Dotted Keys

An inline table is simply a table, and there is no difference in handling it compared to other tables in C++ code.

```toml
a = {b = 42, c = "foo"}
```

Dotted keys are also just tables, with no difference in handling compared to other tables in C++ code.

```toml
a.b = 42
a.c = "foo"
```

These TOML files are identical to the following file.

```toml
[a]
b = 42
c = "foo"
```

Thus, they can all be processed with the exact same code.

```cpp
const auto input = toml::parse("input.toml");

assert(input.at("a").at("b").as_integer() == 42);
assert(input.at("a").at("c").as_string()  == "foo");
```

However, it is possible to distinguish them based on format information.

```cpp
const auto input = toml::parse("input.toml");
switch(input.at("a").as_table_fmt().fmt)
{
    case toml::table_format::oneline: 
    {
        std::cout << "inline table" << std::endl;
        break;
    }
    case toml::table_format::multiline:
    {
        std::cout << "normal table" << std::endl;
        break;
    }
    case toml::table_format::dotted:
    {
        std::cout << "dotted keys" << std::endl;
        break;
    }
}
```

This format information is also considered during serialization, which will be described later.

## Handling Date and Time Information

[`local_date`]({{<ref "docs/reference/datetime#local_date">}}),
[`local_time`]({{<ref "docs/reference/datetime#local_time">}}),
[`local_datetime`]({{<ref "docs/reference/datetime#local_datetime">}}), and
[`offset_datetime`]({{<ref "docs/reference/datetime#offset_datetime">}})
are parsed into dedicated structures with corresponding member variables in toml11.

When using these, you can directly extract the values or use `toml::get` and `toml::find` to convert them into types such as `std::chrono::system_clock::time_point` or `std::tm`.

## Converting Using `toml::get<T>`

`toml::get<T>` is a function that converts and retrieves the value stored in `toml::value`.
Specify the desired target type as `T`.

```cpp
const toml::value v = /*...*/;
std::cout << toml::get<int>(v) << std::endl;
```

The `toml::find<T>` function, described later, also has the same functionality for type conversion.

If an unsupported type is specified for the stored value, a `toml::type_error` is thrown.

### Simple Conversions

#### boolean_type

Conversion from `boolean_type` is possible only to `bool`.

#### integer_type

Any type for which `std::is_integral<T>` is `true`, except `bool`, can be converted from `integer_type`.

```cpp
toml::value v(42);
const auto u32 = toml::get<std::uint32_t>(v);
const auto i16 = toml::get<short>(v);
```

#### floating_type

Any type for which `std::is_floating_point<T>` is `true` can be converted from `floating_type`.

```cpp
toml::value v(3.14);
const auto f64 = toml::get<double>(v);
const auto f32 = toml::get<float >(v);
```

#### string_type

`string_type` can be converted to `std::string`.
From C++17 onwards, it can also be converted to `std::string_view`.

```cpp
toml::value v("foo");
const auto s = toml::get<std::string>(v);

// C++17
const auto sv = toml::get<std::string_view>(v);
```

#### datetime variants

[`local_date`]({{<ref "docs/reference/datetime#local_date">}}),
[`local_datetime`]({{<ref "docs/reference/datetime#local_datetime">}}), and
[`offset_datetime`]({{<ref "docs/reference/datetime#offset_datetime">}}) represent specific dates and times,
so they can be converted to `std::chrono::system_clock::time_point`.

However, since [`local_time`]({{<ref "docs/reference/datetime#local_time">}}) does not include date information, it supports conversion to `std::chrono::duration` as the elapsed time from `00:00.00`.

Additionally, `local_date` and `local_datetime` conversions take the executing machine's timezone into account.

```toml
date = 2024-01-23
time = 12:30:00
l_dt = 2024-01-23T12:30:00
o_dt = 2024-01-23T12:30:00+09:00
```

```cpp
const auto input = toml::parse("input.toml");

const auto date = toml::get<std::chrono::system_clock::time_point>(input.at("date"));
const auto l_dt = toml::get<std::chrono::system_clock::time_point>(input.at("l_dt"));
const auto o_dt = toml::get<std::chrono::system_clock::time_point>(input.at("o_dt"));

const auto time = toml::get<std::chrono::minutes>(input.at("time")); // 12 * 60 + 30 min
```

### Conditions for Obtaining References

`toml::get<T>` can return a reference if `T` is the exact type stored in `toml::value`.

Conversely, if a conversion is necessary (e.g., extracting an integer stored as `std::int64_t` into `std::uint32_t`), it is not possible to return a reference to the converted type.

When no conversion is needed, the returned reference can be used to modify the value.

```cpp
toml::value v(42);

toml::get<toml::value::integer_type>(v) = 6 * 9;

assert(v.as_integer() == 54);
```

### Converting Arrays to STL Containers

If all elements in an array have the same type and can be converted to `T`, it is possible to convert them to `std::vector<T>`.

```toml
a = [1, 2, 3, 4, 5]
```

```cpp
const auto a = toml::get<std::vector<int>>(input.at("a"));
```

Other STL containers can also be used.

```cpp
const auto a1 = toml::get<std::deque<int>>(input.at("a"));
const auto a2 = toml::get<std::list <int>>(input.at("a"));
const auto a3 = toml::get<std::array<int, 5>>(input.at("a"));
```

When converting to `std::array`, the number of elements must match. If they don't match, a `std::out_of_range` exception is thrown.

Non-STL containers that have a default constructor and a `push_back` method can also be converted using `toml::get`.

```cpp
const auto a = toml::get<boost::container::small_vector<int, 8>>(input.at("a"));
```

### Converting Arrays to `std::pair` or `std::tuple`

If an array contains elements of different types, it can be converted to `std::pair` or `std::tuple`.

```toml
a = [true, 3.14]
b = [42, 2.718, "foo"]
```

```cpp
const auto a = toml::get<std::pair<bool, double>>(input.at("a"));
const auto b = toml::get<std::tuple<int, double, std::string>>(input.at("b"));
```

As with `std::array`, the length of the array must match the number of elements in the `std::pair` or `std::tuple`. If they don't match, a `std::out_of_range` exception is thrown.

Additionally, each element must be convertible to the corresponding element in the `std::pair` or `std::tuple`. If conversion is not possible, a `toml::type_error` exception is thrown.

### Converting Nested Arrays

Nested arrays can be converted to nested containers.

```toml
a = [ [1, 2, 3], [4, 5, 6] ]
```

```cpp
const auto a = toml::get<std::vector<std::vector<int>>>(input.at("a"));
```

If the types are different, `std::pair` or `std::tuple` can be useful.

```toml
a = [ [1, 2, 3], ["foo", "bar"] ]
```

```cpp
const auto a = toml::get<
    std::pair<std::vector<int>, std::vector<std::string>>
    >(input.at("a"));
```

### Converting Tables to `std::map`

If all values in a table have the same type, they can be converted to `std::map` or `std::unordered_map`.

```toml
t = {a = 1, b = 2}
```

```cpp
const auto t = toml::get<std::map<std::string, int>>(input.at("t"));
```

Non-STL containers that have a default constructor and an `emplace(key, mapped)` method can also be converted using `toml::get`.

```cpp
const auto t = toml::get<boost::container::flat_map<std::string, int>>(input.at("t"));
```

If the conversion of any element fails, a `toml::type_error` exception is thrown.

## Using `toml::get_or` to Specify a Value on Failure

`toml::get` throws a `toml::type_error` exception if the conversion fails.

By using `toml::get_or`, you can specify a default value to return instead of an exception in case of a conversion failure.

Unlike `toml::get<T>`, `get_or` infers the target type from the arguments, so there is no need to specify `<T>`.

```cpp
const auto a = toml::get_or(input.at("a"), 42);
```

The types that can be converted are the same as for `toml::get`.

If you specify `toml::value::xxx_type`, you can also retrieve a reference, but in that case, the argument must also be a reference.

```cpp
toml::value::integer_type a_default = 42;
auto a& = toml::get_or(input.at("a"), a_default);
```

## Using `toml::find<T>` to Search and Convert Simultaneously

`toml::find<T>` is a function that searches for a value in a `toml::value` holding a table and simultaneously performs the same type conversion as `toml::get`.

```cpp
const auto a = toml::find<int>(input, "a");
// Equivalent to: const auto a = toml::get<int>(input.at("a"));
```

`toml::find<T>` can also be used with arrays.

```cpp
const auto a  = input.at("a");
const auto a2 = toml::find<int>(a, 2);
// Equivalent to: const auto a2 = toml::get<int>(input.at("a").at(2));
```

If an error occurs during type conversion, `toml::find<T>` throws the same `toml::type_error` as `toml::get`.
If the key is not found or the index does not exist, it throws `std::out_of_range`.

If no type is specified, `toml::find` returns a `toml::value` without type conversion.

```cpp
const auto a = toml::find(input, "a");
// Equivalent to: const auto a = input.at("a");
```

`toml::find<T>` can access values recursively.

```toml
a = {b = {c = 42}}
```

```cpp
const auto a_b_c = toml::find<int>(input, "a", "b", "c");
// Equivalent to: const auto a = toml::get<int>(input.at("a").at("b").at("c"));
```

You can mix keys and indexes.


```toml
a = [ {b = 1}, {b = 2}, {b = 3} ]
```

```cpp
const auto a_2_b = toml::find<int>(input, "a", 2, "b");
// Equivalent to: const auto a = toml::get<int>(input.at("a").at(2).at("c"));
```

{{<hint info>}}

TOML supports a feature called quoted keys, which allows using normally disallowed characters in keys by enclosing them in `""` or `''`. Within quoted keys, `.` does **not** introduce tables.

```toml
"127.0.0.1" = "value"
site."google.com" = true
```

You can read this TOML file as follows:

```cpp
const auto input = toml::parse("input.toml");

assert(input.at("127.0.0.1").as_string() == "value");
assert(input.at("site").at("google.com").as_boolean());
```

To handle such cases seamlessly, toml11 does not automatically split keys containing `.`. Explicitly specifying the table hierarchy helps in structuring the input file correctly.

Reference: [toml.io/Key](https://toml.io/en/v1.0.0#key)

{{</hint>}}

## Using `toml::find_or` to Search and Specify a Value on Failure

`toml::find_or` works similarly to `toml::find<T>`, but allows specifying a default value to return if the search or conversion fails.

This is useful when you want to provide a fallback value instead of handling exceptions.

```cpp
const auto a = toml::find_or(input, "a", 42);
```

## Using `toml::find_or_default` to Search and Use the Default Value on Failure

`toml::find_or_default` works similarly to `toml::find_or<T>` but returns the default constructor result if the search or conversion fails.

This is useful when you want to use the default value instead of handling exceptions, especially when the default construction is costly, as this delays it until the actual failure happens.

```cpp
const auto a = toml::find_or(input, "a", expensive());  // ctor is called before the function call
const auto a = toml::find_or_default<expensive>(input, "a");  // ctor will be called only on failure
```

## `toml::find<std::optional<T>>`

If `std::optional` is available, you can specify `std::optional` as a template argument of `toml::find`.

Recursive access is also supported.

```cpp
const auto input = toml::parse_str(R"(
integer = 1

[table]
key = 2

[[array-of-tables]]
key = 3
)");

const auto a = toml::find<std::optional<int>>(input, "integer");
const auto b = toml::find<std::optional<int>>(input, "table", "key");
const auto c = toml::find<std::optional<int>>(input, "array-of-tables", 0, "key");
```

If a key does not exist, no exception is thrown, and `std::nullopt` is returned.

However, if a type conversion fails, or if you attempt to access a key on a value that is not a table, or an index on a value that is not an array, a `toml::type_error` is thrown.

## Defining Conversions for User-Defined Types

With `toml::get` and `toml::find`, you can use user-defined types by employing one of the following methods.

### Defining `toml::from`

In toml11, there is a `toml::from` type that supports conversions from user-defined types by specializing it as follows:

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib

namespace toml
{
template<>
struct from<extlib::foo>
{
    static extlib::foo from_toml(const toml::value& v)
    {
        return extlib::foo{
            toml::find<int>(v, "a"),
            toml::find<std::string>(v, "b")
        };
    }
};
} // toml
```

If you also want to support `toml::value` with a modified type configuration, do as follows:

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib

namespace toml
{
template<>
struct from<extlib::foo>
{
    template<typename TC>
    static extlib::foo from_toml(const toml::basic_value<TC>& v)
    {
        return extlib::foo{
            toml::find<int>(v, "a"),
            toml::find<std::string>(v, "b")
        };
    }
};
} // toml
```

This definition can be automatically generated using `TOML11_DEFINE_CONVERSION_NON_INTRUSIVE`.

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib

TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(extlib::foo, a, b)
```

Alternatively, you can use a reflection library. Refer to the sample in `example` using `boost-ext/reflect`.

### Defining a `from_toml` Member Function

You can also define a conversion by defining a `from_toml` member function.

When using this method, a default constructor is required.

```cpp
struct bar
{
    int a;
    std::string b;

    void from_toml(const toml::value& v)
    {
        this->a = toml::find<int>(v, "a");
        this->b = toml::find<std::string>(v, "b");
        return ;
    }
};
```

If both are defined, `toml::from` takes precedence.

### Constructor Accepting `toml::value`

If there is a constructor that accepts `toml::value`, conversion via `toml::get` can be performed.

```cpp
struct baz
{
    explicit baz(const toml::value& v)
        : a(toml::find<int>(v, "a")), b(toml::find<std::string>(v, "b"))
    {}
    int a;
    std::string b;
};
```

If both are defined, `toml::from` and `from_toml` take precedence.

## Applying Functions with `toml::visit`

If you have a function object that can be applied to all types stored in `toml::value`, you can directly call that function without type conversion using `toml::visit`.

```cpp
struct type_name_of
{
    std::string operator()(const toml::value::boolean_type        &) const {return "boolean";}
    std::string operator()(const toml::value::integer_type        &) const {return "integer";}
    std::string operator()(const toml::value::floating_type       &) const {return "floating";}
    std::string operator()(const toml::value::string_type         &) const {return "string";}
    std::string operator()(const toml::value::local_time_type     &) const {return "local_time";}
    std::string operator()(const toml::value::local_date_type     &) const {return "local_date";}
    std::string operator()(const toml::value::local_datetime_type &) const {return "local_datetime";}
    std::string operator()(const toml::value::offset_datetime_type&) const {return "offset_datetime";}
    std::string operator()(const toml::value::array_type          &) const {return "array";}
    std::string operator()(const toml::value::table_type          &) const {return "table";}
};

toml::value v(3.14);
std::cout << toml::visit(type_name_of{}, v) << std::endl; // floating
```

## Constructing `toml::value`

`toml::value` can be constructed not only internally by the parser but also in user code.

You can construct it by passing a type that is either the same as or convertible to the types stored in `toml::value`.

```cpp
toml::value v1(true);
toml::value v2(42);
toml::value v3(3.14);
```

For arrays, you can use `toml::array`:

```cpp
toml::value v(toml::array{1, 2, 3});
```

Alternatively, you can pass containers such as `std::vector` directly:

```cpp
const std::vector<toml::value> a{1,2,3};
toml::value v(a);
```

For tables, you can use `toml::table`:

```cpp
toml::value v(toml::table{{"foo", 1}, {"bar", 2}, {"baz", 3}});
```

Or pass containers such as `std::map` directly:

```cpp
const std::map<std::string, toml::value> t{
        {"foo", 1}, {"bar", 2}, {"baz", 3}
    }
toml::value v(t);
```

You can pass `format_info` and comments to the constructor.

The type of comments is `std::vector<std::string>`.
Each element corresponds to a line.

```cpp
toml::integer_format_info fmt;
fmt.fmt = toml::integer_format::hex;
fmt.spacer = 4;

toml::value v1(0xDEADBEEF, fmt);
toml::value v2(0xC0FFEE, fmt, {"hex value!"});
```

## Converting to `toml::value`

When constructing `toml::value` from user-defined types, you can customize the behavior by defining `toml::into` or `into_toml`.

`toml::into` is particularly useful when converting types from other libraries.

### Defining `toml::into`

By specializing `toml::into`, you can enable conversions to `toml::value`.

This is useful for types from external libraries that do not provide a conversion to `toml::value`.

Since `toml::value` passes `type_config` during conversion, you need to accept the `template` argument of `basic_value`.

```cpp
namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
} // extlib

template<>
struct into<extlib::foo>
{
    template<typename TC>
    static toml::basic_value<TC> into_toml(const extlib::foo& f)
    {
        return toml::basic_value<TC>(typename toml::basic_value<TC>::table_type{{"a", f.a}, {"b", f.b}});
    }
};
```

### Defining `into_toml` Member Function

Similar to `from_toml`, you can define the conversion through a member function.

If `toml::into` is defined, it takes precedence.

```cpp
struct bar
{
    int a;
    std::string b;

    template<typename TC>
    toml::basic_value<TC> into_toml() const
    {
        return toml::basic_value<TC>(typename toml::basic_value<TC>::table_type{
                {"a", this->a}, {"b", this->b}
            });
    }
};
```

# Checking Whether a Value Has Been Accessed

{{% hint warning %}}

This feature is not enabled by default. To use it, you need to define `TOML11_ENABLE_ACCESS_CHECK`.
Additionally, since this feature introduces extra processing on parsed values, it may impact runtime performance.

{{% /hint %}}

When compiled with the `TOML11_ENABLE_ACCESS_CHECK` macro defined, the `toml::value` class gains an additional method: `bool accessed() const`.
This allows you to check whether a value has been accessed after parsing.

```console
$ g++ -std=c++17 -O2 -DTOML11_ENABLE_ACCESS_CHECK -I/path/to/toml11/include main.cpp
```

or

```console
$ cmake -B ./build -DTOML11_ENABLE_ACCESS_CHECK=ON
```

or

```cmake
CPMAddPackage(
    NAME toml11
    GITHUB_REPOSITORY "ToruNiina/toml11"
    VERSION 4.4.0
    OPTIONS "CMAKE_CXX_STANDARD 17" "TOML11_PRECOMPILE ON" "TOML11_ENABLE_ACCESS_CHECK ON"
    )
```

This feature allows users to implement code that warns about values defined in a table but never used.

```cpp
#include <toml.hpp>

namespace yours
{

Config read_config(const toml::value& input)
{
    const auto cfg = read_your_config(input);

    for (const auto& [k, v] : input.as_table())
    {
        if (!v.accessed())
        {
            std::cerr << toml::format_error("value defined but not used",
                v.source_location(), "not used");
        }
    }
    return cfg;
}
} // namespace yours
```

This feature is useful when a value is mistakenly defined under the wrong name but is never accessed. For example:

```toml
# The correct key is "reactions"
# reactions = [ ":+1:", "star" ]

# This key is incorrect and will not be read
reaction = [ ":+1:", "star" ]
```

If this file is read using the above code, `read_your_config` will search for `reactions`. Since it is not defined, it will process `reactions` as an empty array.
In this case, `input.at("reaction").accessed()` will be `false`, allowing it to be detected as an error.



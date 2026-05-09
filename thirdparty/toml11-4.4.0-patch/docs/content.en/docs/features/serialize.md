+++
title = "serializing values"
type  = "docs"
weight = 40
+++

# Outputting TOML Files

Using `toml::format`, you can convert a `toml::value` to a string.

```cpp
#include <toml.hpp>
#include <cassert>

int main()
{
    const toml::value v(toml::table{
        {"a", 42},
        {"b", "foo"},
    });
    const std::string s = toml::format(v);

    const toml::value u = toml::parse_str(s);

    assert(u.at("a").as_integer() == 42);
    assert(u.at("b").as_string()  == "foo");

    return 0;
}
```

If the `toml::value` contains a `table_type`, it is interpreted as the root table of the file.

If a `toml::value` containing anything other than `table_type` is passed, only that value is formatted.

Certain format specifications may require a key to be provided for formatting. For example, `toml::array_format::array_of_tables` formats as `[[array.of.tables]]`, which requires key access.

If a format specification that requires a key is provided without a key, a `toml::serialization_error` is thrown.

Additionally, if there are values that contradict the format specification, a `toml::serialization_error` is thrown. For instance, specifying `integer_format::hex` for a negative integer, or `string_format::literal` for a string containing newlines, will cause an error.

The method for specifying formats is explained later.

## Outputting with Keys

You can pass a key to `toml::format` as a `std::string`.

In this case, the key is considered to be under the root table, and the passed value corresponds to that key.

For nested keys, you can pass a `std::vector<std::string>`.

```cpp
#include <toml.hpp>
#include <cassert>

int main()
{
    const toml::value v(toml::table{
        {"a", 42},
        {"b", "foo"},
    });
    const std::string s = toml::format("bar", v);

    const toml::value u = toml::parse_str(s);

    assert(u.at("bar").at("a").as_integer() == 42);
    assert(u.at("bar").at("b").as_string()  == "foo");

    return 0;
}
```

## Specifying Formats

Each type in `toml::value` has a corresponding format information type.

For `toml::value::integer_type`, there is `toml::integer_format_info`.
For `toml::value::table_type`, there is `toml::table_format_info`.

These format information types are set when parsing and are retained even if the value is changed, as long as the type remains the same.

You can access and directly edit these formats using member functions like `as_integer_fmt()` or `as_table_fmt()`.

Below are some examples explaining how to use these formats.

For more details on how to access formats, refer to the [`toml::value` reference]({{< ref "/docs/reference/value" >}}). For a complete list and detailed information on format information classes, see the [format reference]({{< ref "/docs/reference/format" >}}).

### Specifying Integer Formats

For integers, you can specify the radix, width, and the position of `_`.

When using `hex`, `oct`, or `bin`, values are padded with zeros until the specified width is reached. For `dec`, the width specification adds spaces, which are not parsed.

For more details, see the [integer format reference]({{< ref "/docs/reference/format#integer_format" >}}).

### Single-Line and Multi-Line Arrays

For arrays, you can specify `toml::array_format::oneline` or `toml::array_format::multiline`.

```toml
# oneline
a = [1, 2, 3, 4, 5]
# multiline
a = [
  1,
  2,
  3,
  4, 
  5
]
```

When using `multiline`, you can specify the indentation. Each element is indented by the amount specified in `body_indent`, and the closing bracket `]` is indented by the amount specified in `closing_indent`.

The type of character used for indentation is specified by `indent_type`, and you can choose between `toml::indent_char::space` or `toml::indent_char::tab`.

{{<hint warning>}}
Ensure that the same type of character is used for indentation throughout the document.

If different types of characters are specified for indentation within the same file, the result is undefined. Some form of indentation will be applied, but the type of character and the depth of the indentation may be inconsistent.
{{</hint>}}

If all elements of an `array` have `table_type`, you can specify `toml::array_format::array_of_tables`.

If you do not specify `array_of_tables` and use `multiline`, the tables will be formatted as inline tables.

```toml
# multiline
a = [
  {foo = 42},
  {bar = "hoge"},
]

# array_of_tables
[[a]]
foo = 42

[[a]]
bar = "hoge"
```

By default, `toml::array_format::default_format` is used. This automatically selects an appropriate format.

For example, with `default_format`, if all elements are `table_type`, it will choose `array_of_tables`. Short arrays are formatted as `oneline`, while long or nested arrays, or those with complex elements, are formatted as `multiline`.

For more details, see the [array format reference]({{< ref "/docs/reference/format#array_format" >}}).

### Inline Tables

To format a table as an inline table, specify `toml::table_format::oneline`.
For standard tables, use `toml::table_format::multiline`.

```toml
oneline = {a = 42, b = "foo"}

[multiline]
a = 42
b = "foo"
```

In TOML v1.1.0, line breaks within inline tables are allowed. In this case, use `toml::table_format::multiline_oneline`. This is only applied if the corresponding feature flag is set to `true` as per the TOML version specification described later.

```toml
multiline_oneline = {
    a = 42,
    b = "foo"
}
```

For more details, see the [table format reference]({{< ref "/docs/reference/format#table_format" >}}).

## Specifying the TOML Language Version for Output

Certain language features, such as line breaks within inline tables and `\x` escape sequences, are only available after TOML v1.1.0.

The `toml::format` function accepts a `toml::spec` as its argument.

This allows you to specify the version of TOML to use during serialization.

When you use `toml::parse` with a `toml::spec` to leverage new features,
the parsed values may contain format information that is only compatible with that specific version.
Ensure that you pass the same `toml::spec` to `toml::format` to maintain compatibility.

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    const auto spec = toml::spec::v(1, 1, 0);

    const toml::value v = toml::parse("input.toml", spec);

    std::cout << toml::format(v, spec) << std::endl;

    return 0;
}
```

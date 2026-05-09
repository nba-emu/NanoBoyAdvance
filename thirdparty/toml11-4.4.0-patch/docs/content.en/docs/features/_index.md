+++
title = "features"
type  = "docs"
weight = 2
bookCollapseSection = true
+++

# Features

This section explains the main features provided by toml11, with examples.

## [Parsing Files and Strings](parsing_files)

Describes the functions for parsing files and strings, and how to handle the errors they produce.

Includes:

- Parsing files
- Parsing strings
- Parsing byte arrays
- Parsing files without throwing exceptions
- Parsing strings without throwing exceptions
- Parsing byte arrays without throwing exceptions

## [Extracting Values from `toml::value`](value)

Explains how to examine, extract, and convert the data types held by `toml::value`.

Includes:

- Checking the type of a value using member functions
- Accessing values using member functions
- Accessing comments
- Handling inline tables and dotted keys
- Handling date information
- Using `toml::get<T>` for conversion
- Using `toml::get_or` to specify a fallback value
- Using `toml::find<T>` for searching and conversion
- Using `toml::find_or` to specify a fallback value
- Defining conversions with user-defined types
- Applying functions with `toml::visit`
- Constructing `toml::value`

## [Creating Error Messages](error_message)

Explains how to generate error messages with location information from a TOML file using `toml::value`.

Includes:

- Extracting location information from `toml::value`
- Constructing error messages
- Adding color to the output

## [Serializing TOML Files](serialize)

Describes how to format the values of `toml::value` and the available formatting options.

Includes:

- Specifying formats for each value of `toml::value`
- Formatting `toml::value` into a string

## [Configuring Types of `toml::value`](configure_types)

Explains how to customize the types stored in `toml::value` (such as `integer_type` and `table_type`).

Includes:

- Defining `type_config`
- Using `ordered_type_config`
- Disabling comment preservation
- Using different containers like `std::deque`
- Using different numeric types like `boost::multiprecision`

## [TOML Literals](literal)

Explains the `_toml` literal for embedding TOML files directly in C++ code.

Includes:

- Using TOML literals

## [TOML Language Version](toml_spec)

Describes the versions of the TOML language supported by toml11 and how to control language features added in TOML-v1.1.0.

Includes:

- Using TOML language version 1.1.0
- Using specific features of TOML language version 1.1.0

## [TOML Language Extensions](extension)

Explains the custom extensions to the TOML language provided by toml11.

Includes:

- Supporting `null`
- Supporting hexadecimal format for floating-point numbers
- Allowing units for numbers

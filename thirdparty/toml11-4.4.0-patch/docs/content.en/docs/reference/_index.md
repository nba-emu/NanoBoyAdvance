+++
title = "reference"
type  = "docs"
weight = 3
bookCollapseSection = true
+++

# Reference

Here, we explain the effects of the classes and functions provided by toml11.

## Directory Structure

`toml.hpp` and `toml_fwd.hpp` reside in `${TOML11_INCLUDE_DIR}`.
Other files are located in `${TOML11_INCLUDE_DIR}/toml11`.

If you want to `#include` each feature's file individually, use `#include <toml11/color.hpp>`.
If you want to include all at once, use `#include <toml.hpp>`.

## [color.hpp](color)

Defines functions related to colorizing error messages.

## [comments.hpp](comments)

Defines types `preserve_comment` and `discard_comment` for preserving comments.

## [conversion.hpp](conversion)

Defines macros to automatically convert `toml::value` and user-defined classes.

## [datetime.hpp](datetime)

Defines classes for datetime information.

## [error_info.hpp](error_info)

Defines a class for error information.

## [exception.hpp](exception)

Defines the base class for exceptions used in toml11, `toml::exception`.

## [find.hpp](find)

Defines the `toml::find` function to search for and convert values.

## [format.hpp](format)

Defines classes for formatting information of values.

## [from.hpp](from)

Forward declaration of the `from<T>` type for converting user-defined types.

## [get.hpp](get)

Defines the `toml::get<T>` function to retrieve and convert values from `toml::value`.

## [into.hpp](into)

Forward declaration of the `into<T>` type for converting user-defined types.

## [literal.hpp](literal)

Defines the `operator"" _toml` literal.

## [ordered_map.hpp](ordered_map)

Defines `toml::ordered_map`.

## [parser.hpp](parser)

Defines functions to parse files or strings.

## [result.hpp](result)

Defines the `result<T, E>` type for representing success or failure values used as return types in other functions.

## [serializer.hpp](serializer)

Defines the `toml::format` function and `toml::serializer` used for serialization.

## [source_location.hpp](source_location)

Defines the `source_location` type used for error information, pointing to a location within a file.

## [spec.hpp](spec)

Defines the `toml::semantic_version` and `toml::spec` types to control TOML language version information and feature flags.

## [toml.hpp](toml)

`toml.hpp` includes all other headers, making all toml11 features available.

## [toml_fwd.hpp](toml_fwd)

`toml_fwd.hpp` contains forward declarations of structs defined in toml11 and macro definitions.

## [types.hpp](types)

Defines the `toml::type_config` type for controlling the types held by `toml::value`.

## [value.hpp](value)

Defines the `toml::value` type.

## [value_t.hpp](value_t)

Defines the `toml::value_t` enumeration.

## [version.hpp](version)

Defines the version information for toml11.

## [visit.hpp](visit)

Defines the `toml::visit` function to apply functions to the values held by `toml::value`.

## Notes

Functions not explicitly mentioned here (mostly those defined under `namespace toml::detail` or `namespace toml::cxx`) are available by inspecting the source code but are not guaranteed to maintain their interface across future versions (including patch version updates).

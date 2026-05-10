+++
title = "toml_fwd.hpp"
type  = "docs"
+++

# toml_fwd.hpp

`toml_fwd.hpp` contains forward declarations of structures defined in toml11 and macro definitions.

When only forward declarations of toml11 structures are needed and implementation is not required, including `toml_fwd.hpp` instead of `toml.hpp` can reduce compilation time.

{{<hint warning>}}

Since this file only contains forward declarations, you cannot use `toml::table`, defined as `toml::basic_value<toml::type_config>::table_type`, and similarly defined `toml::array`. This is because they require the implementation of `basic_value`.

{{</hint>}}

This header file and `toml.hpp` are located under `${TOML11_INCLUDE_DIR}/`, while other header files are located under `${TOML11_INCLUDE_DIR}/toml11/`.

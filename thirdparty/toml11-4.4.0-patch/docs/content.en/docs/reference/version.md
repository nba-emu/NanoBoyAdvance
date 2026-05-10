+++
title = "version.hpp"
type  = "docs"
+++

# version.hpp

In `version.hpp`, macros related to the version information of toml11 are defined.

## Macros

### `TOML11_VERSION_MAJOR`

The major version of toml11.

### `TOML11_VERSION_MINOR`

The minor version of toml11.

### `TOML11_VERSION_PATCH`

The patch version of toml11.

## Function

### `license_notice`

```cpp
namespace toml
{
const char* license_notice() noexcept;
}
```

Returns the license notice.

Provided for convenience when redistributing without source code.

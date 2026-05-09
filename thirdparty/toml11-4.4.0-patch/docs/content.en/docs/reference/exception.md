+++
title = "exception.hpp"
type  = "docs"
+++

# exception.hpp

# `toml::exception`

Base class for exception types defined in toml11.

```cpp
namespace toml
{
struct exception : public std::exception
{
  public:
    virtual ~exception() noexcept override = default;
    virtual const char* what() const noexcept override {return "";}
};
} // toml
```

## Member Functions

### Destructor

```cpp
virtual ~exception() noexcept override = default;
```

Override when derived.

### `what`

```cpp
virtual const char* what() const noexcept override {return "";}
```

Returns the error message. Override when derived.

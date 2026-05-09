+++
title = "serializer.hpp"
type  = "docs"
+++

# serializer.hpp

## `format`

Serializes the data.

```cpp
namespace toml
{
template<typename TC>
std::string format(const basic_value<TC>& v,
                   const spec s = spec::default_version());
template<typename TC>
std::string format(const typename basic_value<TC>::key_type& k,
                   const basic_value<TC>& v,
                   const spec s = spec::default_version());
template<typename TC>
std::string format(const std::vector<typename basic_value<TC>::key_type>& ks,
                   const basic_value<TC>& v,
                   const spec s = spec::default_version());
}
```

If there's a conflict between the format information and the `spec`, for example, when using `v1.0.0` with `table_format::multiline_oneline`, the `spec` takes precedence.

### `format(v, spec)`

Formats a `toml::value` according to its format information and the provided `spec`.

If it's a `table_type`, it's formatted as if it were the root table. Otherwise, only the value is formatted.

### `format(k, v, spec)`

Formats a `toml::value` along with the given key.

`v` is interpreted as being defined under that key.

### `format([k,...], v, spec)`

`v` is interpreted as being defined under those keys.
If multiple keys are provided, it's interpreted as a recursively defined table.

## `serialization_error`

Reports errors that occurred during serialization.

```cpp
namespace toml
{
struct serialization_error final : public ::toml::exception
{
  public:
    explicit serialization_error(std::string what_arg, source_location loc);
    ~serialization_error() noexcept override = default;

    const char* what() const noexcept override;
    source_location const& location() const noexcept;
};
}
```

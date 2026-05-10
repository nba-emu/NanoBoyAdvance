# container

Use `boost::container::vector` and `boost::unordered_flat_map` as `array_type` and `table_type`.

## build

Install [boost](https://boost.org), especially after 1.81.0 that supports `unordered_flat_map`.

Then, build toml11 with `-DTOML11_BUILD_EXAMPLES=ON`

```cpp
$ cmake -B ./build/ -DTOML11_BUILD_EXAMPLES=ON
```

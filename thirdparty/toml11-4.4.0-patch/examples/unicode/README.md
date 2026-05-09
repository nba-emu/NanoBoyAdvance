# unicode

Compare TOML key after NFC canonicalization.

It depends on [uni-algo](https://github.com/uni-algo/uni-algo.git).

The example contains two keys that are different in bytewise comparison, but becomes the same after NFC normalization.

## build

Build toml11 with `-DTOML11_BUILD_EXAMPLES=ON`.

```cpp
$ cmake -B ./build/ -DTOML11_BUILD_EXAMPLES=ON
```

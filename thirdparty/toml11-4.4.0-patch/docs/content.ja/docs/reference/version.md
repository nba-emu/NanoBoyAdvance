+++
title = "version.hpp"
type  = "docs"
+++

# version.hpp

`version.hpp`では、toml11とC++のバージョン情報に関係するマクロが定義されます。

## マクロ

### `TOML11_VERSION_MAJOR`

toml11のメジャーバージョンです。

### `TOML11_VERSION_MINOR`

toml11のマイナーバージョンです。

### `TOML11_VERSION_PATCH`

toml11のパッチバージョンです。

## 関数

### `license_notice`

```cpp
namespace toml
{
const char* license_notice() noexcept;
}
```

ライセンス条項を返します。

ソースコードを公開せずに頒布する際の利便性のために用意されています。

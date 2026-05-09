+++
title = "exception.hpp"
type  = "docs"
+++

# exception.hpp

# `toml::exception`

toml11で定義される例外型の基底クラスです。

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

## メンバ関数

### デストラクタ

```cpp
virtual ~exception() noexcept override = default;
```

派生する際に上書きします。

### `what`

```cpp
virtual const char* what() const noexcept override {return "";}
```

エラーメッセージを返します。派生する際に上書きします。


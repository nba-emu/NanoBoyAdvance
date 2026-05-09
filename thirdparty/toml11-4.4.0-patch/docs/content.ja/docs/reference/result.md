+++
title = "result.hpp"
type  = "docs"
+++

# result.hpp

`result.hpp`は、成功値か失敗値かのどちらかを持つ`result`型を定義します。

これは、例外を投げない`toml::try_parse`の返り値として使用されます。

# success

成功値を持つ型です。

```cpp
namespace toml
{
template<typename T>
struct success
{
    using value_type = T;

    explicit success(value_type v);

    ~success() = default;
    success(const success&) = default;
    success(success&&)      = default;
    success& operator=(const success&) = default;
    success& operator=(success&&)      = default;

    template<typename U>
    explicit success(U&& v);
    template<typename U>
    explicit success(success<U> v);

    value_type&       get()       noexcept;
    value_type const& get() const noexcept;
};

template<typename T>
success<typename std::decay<T>::type> ok(T&& v);
template<std::size_t N>
success<std::string> ok(const char (&literal)[N])
}
```

## メンバ型

```cpp
using value_type = T;
```

成功値の型です。

## メンバ関数

### コンストラクタ

```cpp
explicit success(value_type v);
```

`value_type`を受け取って構築します。


```cpp
template<typename U>
explicit success(U&& v);
```

`value_type`に変換可能な他の型を受け取って構築します。

```cpp
template<typename U>
explicit success(success<U> v);
```

`value_type`に変換可能な他の型を持つ`success`型を受け取って構築します。

### `get()`

```cpp
value_type&       get()       noexcept;
value_type const& get() const noexcept;
```

値にアクセスします。

## 非メンバ関数

### `ok(T)`

```cpp
template<typename T>
success<typename std::decay<T>::type> ok(T&& v);
template<std::size_t N>
success<std::string> ok(const char (&literal)[N])
```

成功値から`success`型を構築して返します。

文字列リテラルを与えた場合は、`std::string`にして返します。

# `success<reference_wrapper<T>>`

`success`の特殊化。成功値が参照であるときに使用されます。

```cpp
namespace toml
{
template<typename T>
struct success<std::reference_wrapper<T>>
{
    using value_type = T;

    explicit success(std::reference_wrapper<value_type> v) noexcept;

    value_type&       get()       noexcept;
    value_type const& get() const noexcept;
};
}
```

## メンバ型

```cpp
using value_type = T;
```

成功値の型です。参照ではなく、`std::reference_wrapper<T>`の`T`です。

### `get()`

```cpp
value_type&       get()       noexcept;
value_type const& get() const noexcept;
```

値にアクセスします。

# failure

失敗値を持つ型です。

```cpp
namespace toml
{
template<typename T>
struct failure
{
    using value_type = T;

    explicit failure(value_type v);

    ~failure() = default;
    failure(const failure&) = default;
    failure(failure&&)      = default;
    failure& operator=(const failure&) = default;
    failure& operator=(failure&&)      = default;

    template<typename U>
    explicit failure(U&& v);
    template<typename U>
    explicit failure(failure<U> v);

    value_type&       get()       noexcept;
    value_type const& get() const noexcept;
};

template<typename T>
failure<typename std::decay<T>::type> err(T&& v);
template<std::size_t N>
failure<std::string> err(const char (&literal)[N]);
}
```

## メンバ型

```cpp
using value_type = T;
```

失敗値の型です。

## メンバ関数

### コンストラクタ

```cpp
explicit failure(value_type v);
```

`value_type`を受け取って構築します。


```cpp
template<typename U>
explicit failure(U&& v);
```

`value_type`に変換可能な他の型を受け取って構築します。

```cpp
template<typename U>
explicit failure(failure<U> v);
```

`value_type`に変換可能な他の型を持つ`failure`型を受け取って構築します。

### `get()`

```cpp
value_type&       get()       noexcept;
value_type const& get() const noexcept;
```

値にアクセスします。

## 非メンバ関数

### `err(T)`

```cpp
template<typename T>
failure<typename std::decay<T>::type> err(T&& v);
template<std::size_t N>
failure<std::string> err(const char (&literal)[N]);
```

失敗値から`failure`型を構築して返します。

文字列リテラルを与えた場合は、`std::string`にして返します。

# `failure<reference_wrapper<T>>`

`failure`の特殊化。失敗値が参照であるときに使用されます。

```cpp
namespace toml
{
template<typename T>
struct failure<std::reference_wrapper<T>>
{
    using value_type = T;

    explicit failure(std::reference_wrapper<value_type> v) noexcept;

    value_type&       get()       noexcept {return value.get();}
    value_type const& get() const noexcept {return value.get();}
};
}
```

## メンバ型

```cpp
using value_type = T;
```

失敗値の型です。参照ではなく、`std::reference_wrapper<T>`の`T`です。

### `get()`

```cpp
value_type&       get()       noexcept
value_type const& get() const noexcept
```

値にアクセスします。

# result

成功値か失敗値かのどちらかを持つ型です。

```cpp
namespace toml
{
template<typename T, typename E>
struct result
{
    using success_type = success<T>;
    using failure_type = failure<E>;
    using value_type = typename success_type::value_type;
    using error_type = typename failure_type::value_type;

    result(success_type s);
    result(failure_type f);

    template<typename U>
    result(success<U> s);
    template<typename U>
    result(failure<U> f);

    result& operator=(success_type s);
    result& operator=(failure_type f);

    template<typename U>
    result& operator=(success<U> s);
    template<typename U>
    result& operator=(failure<U> f);

    ~result() noexcept;

    result(const result& other);
    result(result&& other);
    result& operator=(const result& other);
    result& operator=(result&& other);

    template<typename U, typename F>
    result(result<U, F> other);
    template<typename U, typename F>
    result& operator=(result<U, F> other);

    bool is_ok()  const noexcept;
    bool is_err() const noexcept;

    explicit operator bool() const noexcept;

    value_type&       unwrap(cxx::source_location loc = cxx::source_location::current());
    value_type const& unwrap(cxx::source_location loc = cxx::source_location::current()) const;

    value_type&       unwrap_or(value_type& opt)             noexcept;
    value_type const& unwrap_or(value_type const& opt) const noexcept;

    error_type&       unwrap_err(cxx::source_location loc = cxx::source_location::current());
    error_type const& unwrap_err(cxx::source_location loc = cxx::source_location::current()) const;

    value_type&       as_ok()       noexcept;
    value_type const& as_ok() const noexcept;

    error_type&       as_err()       noexcept;
    error_type const& as_err() const noexcept;
};
}
```

## メンバ型

### `success_type`

`success<T>`です。

### `failure_type`

`failure<E>`です。

### `value_type`

成功型`T`です。`success_type::value_type`のエイリアスです。

成功型`T`として`std::reference_wrapper<U>`が渡された場合、その引数型の`U`になります。

### `error_type`

失敗型`E`です。`failure_type::value_type`のエイリアスです。

失敗型`E`として`std::reference_wrapper<F>`が渡された場合、その引数型の`F`になります。

## メンバ関数

### コンストラクタ

```cpp
result() = delete;
```

`result`型はデフォルトでは構築できません。
成功型か失敗型を与える必要があります。

```cpp
result(success_type s);
result(failure_type f);
```

成功型、失敗型を受け取って構築します。

```cpp
template<typename U>
result(success<U> s);
template<typename U>
result(failure<U> f);
```

`value_type`と`error_type`に変換可能な成功型、失敗型を受け取って構築します。

```cpp
template<typename U, typename F>
result(result<U, F> other);
template<typename U, typename F>
result& operator=(result<U, F> other);
```

変換可能な成功型、失敗型を持つ`result`から構築します。

### コピー・ムーブコンストラクタ

```cpp
result(const result& other);
result(result&& other);
```

コピー・ムーブ構築可能です。

### `operator=`

```cpp
result& operator=(const result& other);
result& operator=(result&& other);
```

コピー・ムーブ代入可能です。

```cpp
template<typename U>
result& operator=(success<U> s);
template<typename U>
result& operator=(failure<U> f);
```

変換可能な型を持つ成功型、失敗型は代入可能です。

### `is_ok()`

```cpp
bool is_ok()  const noexcept;
```

成功値を持っている場合`true`を、そうでない場合`false`を返します。

### `is_err()`

```cpp
bool is_err() const noexcept;
```

失敗値を持っている場合`true`を、そうでない場合`false`を返します。

### `operator bool()`

```cpp
explicit operator bool() const noexcept;
```

成功値を持っている場合`true`を、そうでない場合`false`を返します。

### `unwrap()`

```cpp
value_type&       unwrap(cxx::source_location loc = cxx::source_location::current());
value_type const& unwrap(cxx::source_location loc = cxx::source_location::current()) const;
```

成功値を取り出します。

成功値を持っていない場合、`toml::bad_result_access`が送出されます。

`std::source_location`または`std::experimental::source_location`が利用可能か、
あるいはコンパイラ拡張によって同等の情報が利用可能な場合、`what()`文字列に
`unwrap()`が発生したソースコードのファイル名と行数が表示されます。

利用可能でない場合、位置情報は表示されません。

### `unwrap_or()`

```cpp
value_type&       unwrap_or(value_type& opt)             noexcept;
value_type const& unwrap_or(value_type const& opt) const noexcept;
```

成功値を持っていた場合それを、持っていなかった場合は引数で与えられた値を返します。

### `unwrap_err()`

```cpp
error_type&       unwrap_err(cxx::source_location loc = cxx::source_location::current());
error_type const& unwrap_err(cxx::source_location loc = cxx::source_location::current()) const;
```

失敗値を取り出します。

失敗値を持っていない場合、`toml::bad_result_access`が送出されます。

`std::source_location`または`std::experimental::source_location`が利用可能か、
あるいはコンパイラ拡張によって同等の情報が利用可能な場合、`what()`文字列に
`unwrap()`が発生したソースコードのファイル名と行数が表示されます。

利用可能でない場合、位置情報は表示されません。

### `as_ok()`

```cpp
value_type&       as_ok()       noexcept;
value_type const& as_ok() const noexcept;
```

チェックなしで成功値を返します。

成功値を持っていなかった場合、その動作は未定義となります。

### `as_err()`

```cpp
error_type&       as_err()       noexcept;
error_type const& as_err() const noexcept;
```

チェックなしで失敗値を返します。

失敗値を持っていなかった場合、その動作は未定義となります。

# bad_result_access

`result`の`unwrap`または`unwrap_err`で失敗した際に送出される例外です。

```cpp
namespace toml
{
struct bad_result_access : public ::toml::exception
{
  public:
    explicit bad_result_access(const std::string& what_arg);
    virtual ~bad_result_access() noexcept override = default;
    virtual const char* what() const noexcept override;
  protected:
    std::string what_;
};
}
```

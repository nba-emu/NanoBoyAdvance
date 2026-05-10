+++
title = "result.hpp"
type  = "docs"
+++

# result.hpp

`result.hpp` defines the `result` type, which can hold either a success value or a failure value.

This is used as the return type for `toml::try_parse`, which does not throw exceptions.

# success

A type that holds a success value.

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

## Member Types

```cpp
using value_type = T;
```

The type of the success value.

## Member Functions

### Constructor

```cpp
explicit success(value_type v);
```

Constructs with a `value_type` argument.

```cpp
template<typename U>
explicit success(U&& v);
```

Constructs with another type that can be converted to `value_type`.

```cpp
template<typename U>
explicit success(success<U> v);
```

Constructs with another `success` type that can be converted to `value_type`.

### `get()`

```cpp
value_type&       get()       noexcept;
value_type const& get() const noexcept;
```

Accesses the stored value.

## Non-Member Functions

### `ok(T)`

```cpp
template<typename T>
success<typename std::decay<T>::type> ok(T&& v);
template<std::size_t N>
success<std::string> ok(const char (&literal)[N]);
```

Constructs and returns a `success` type from a success value.

Converts a string literal into `std::string`.

# `success<reference_wrapper<T>>`

Specialization of `success` for when the success value is a reference.

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

## Member Types

```cpp
using value_type = T;
```

The type of the success value. It is `T` from `std::reference_wrapper<T>`, not the reference itself.

### `get()`

```cpp
value_type&       get()       noexcept;
value_type const& get() const noexcept;
```

Accesses the stored value.

# failure

A type that holds a failure value.

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

## Member Types

```cpp
using value_type = T;
```

The type of the failure value.

## Member Functions

### Constructor

```cpp
explicit failure(value_type v);
```

Constructs with a `value_type` argument.

```cpp
template<typename U>
explicit failure(U&& v);
```

Constructs with another type that can be converted to `value_type`.

```cpp
template<typename U>
explicit failure(failure<U> v);
```

Constructs with another `failure` type that can be converted to `value_type`.

### `get()`

```cpp
value_type&       get()       noexcept;
value_type const& get() const noexcept;
```

Accesses the stored value.

## Non-Member Functions

### `err(T)`

```cpp
template<typename T>
failure<typename std::decay<T>::type> err(T&& v);
template<std::size_t N>
failure<std::string> err(const char (&literal)[N]);
```

Constructs and returns a `failure` type from a failure value.

Converts a string literal into `std::string`.

# `failure<reference_wrapper<T>>`

Specialization of `failure` for when the failure value is a reference.

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

## Member Types

```cpp
using value_type = T;
```

The type of the failure value. It is `T` from `std::reference_wrapper<T>`, not the reference itself.

### `get()`

```cpp
value_type&       get()       noexcept;
value_type const& get() const noexcept;
```

Accesses the stored value.

# result

A type that holds either a success value or a failure value.

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

## Member Types

### `success_type`

`success<T>`.

### `failure_type`

`failure<E>`.

### `value_type`

The type `T` of the success value, alias for `success_type::value_type`.

If `T` is `std::reference_wrapper<U>`, then it is `U`.

### `error_type`

The type `E` of the failure value, alias for `failure_type::value_type`.

If `E` is `std::reference_wrapper<F>`, then it is `F`.

## Member Functions

### Constructor

```cpp
result() = delete;
```

Cannot construct `result` type by default. Needs to be given either a success or failure type.

```cpp
result(success_type s);
result(failure_type f);
```

Constructs with a `success_type` or `failure_type`.

```cpp
template<typename U>
result(success<U> s);
template<typename U>
result(failure<U> f);
```

Constructs with a `success<U>` or `failure<U>` that is convertible to `value_type` or `error_type`.

```cpp
template<typename U, typename F>
result(result<U, F> other);
template<typename U, typename F>
result& operator=(result<U, F> other);
```

Constructs from or assigns to another `result` with convertible `success` or `failure` types.

### Copy and Move Constructors

```cpp
result(const result& other);
result(result&& other);
```

Can be copy or move constructed.

### `operator=`

```cpp
result& operator=(const result& other);
result& operator=(result&& other);
```

Can be copy or move assigned.

```cpp
template<typename U>
result& operator=(success<U> s);
template<typename U>
result& operator=(failure<U> f);
```

Can be assigned from convertible `success` or `failure` types.

### `is_ok()`

```cpp
bool is_ok()  const noexcept;
```

Returns `true` if it holds a success value, `false` otherwise.

### `is_err()`

```cpp
bool is_err() const noexcept;
```

Returns `true` if it holds a failure value, `false` otherwise.

### `operator bool()`

```cpp
explicit operator bool() const noexcept;
```

Returns `true` if it holds a success value, `false` otherwise.

### `unwrap()`

```cpp
value_type&       unwrap(cxx::source_location loc = cxx::source_location::current());
value_type const& unwrap(cxx::source_location loc = cxx::source_location::current()) const;
```

Returns the stored success value.

Throws `toml::bad_result_access` if it holds a failure value.

If `std::source_location` or equivalent compiler extension is available, the file name and line number where `unwrap()` occurred are included in the `what()` string.

### `unwrap_or()`

```cpp
value_type&       unwrap_or(value_type& opt)             noexcept;
value_type const& unwrap_or(value_type const& opt) const noexcept;
```

Returns the stored success value if present, otherwise returns the provided default value.

### `unwrap_err()`

```cpp
error_type&       unwrap_err(cxx::source_location loc = cxx::source_location::current());
error_type const& unwrap_err(cxx::source_location loc = cxx::source_location::current()) const;
```

Returns the stored failure value.

Throws `toml::bad_result_access` if it holds a success value.

If `std::source_location` or equivalent compiler extension is available, the file name and line number where `unwrap_err()` occurred are included in the `what()` string.

### `as_ok()`

```cpp
value_type&       as_ok()       noexcept;
value_type const& as_ok() const noexcept;
```

Returns the success value without checking.

Behavior is undefined if it holds a failure value.

### `as_err()`

```cpp
error_type&       as_err()       noexcept;
error_type const& as_err() const noexcept;
```

Returns the failure value without checking.

Behavior is undefined if it holds a success value.

# bad_result_access

An exception thrown when `unwrap` or `unwrap_err` fails in a `result`.

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

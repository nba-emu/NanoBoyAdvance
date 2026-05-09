+++
title = "comments.hpp"
type  = "docs"
+++

# comments.hpp

In `comments.hpp`, comment containers are provided.

# `toml::preserve_comments`

`preserve_comments` is a container that preserves comments.

It has all the member functions of `std::vector<std::string>`.

Comments are preserved as `std::string`.
If the comment does not start with `#`, it will be prefixed with `#` during output.
However, this prefixing is not done when adding comments to the container.

Spaces are not automatically added, so if you want a space immediately after `#`,
either start the comment with a space or pass the comment with `#`.

```cpp
namespace toml
{
class preserve_comments;

bool operator==(const preserve_comments&, const preserve_comments&);
bool operator!=(const preserve_comments&, const preserve_comments&);
bool operator< (const preserve_comments&, const preserve_comments&);
bool operator<=(const preserve_comments&, const preserve_comments&);
bool operator> (const preserve_comments&, const preserve_comments&);
bool operator>=(const preserve_comments&, const preserve_comments&);

void swap(preserve_comments&,        preserve_comments&);
void swap(preserve_comments&,        std::vector<std::string>&);
void swap(std::vector<std::string>&, preserve_comments&);

std::ostream& operator<<(std::ostream&, const preserve_comments&);
} //toml
```

## Member types

```cpp
using container_type         = std::vector<std::string>;
using size_type              = container_type::size_type;
using difference_type        = container_type::difference_type;
using value_type             = container_type::value_type;
using reference              = container_type::reference;
using const_reference        = container_type::const_reference;
using pointer                = container_type::pointer;
using const_pointer          = container_type::const_pointer;
using iterator               = container_type::iterator;
using const_iterator         = container_type::const_iterator;
using reverse_iterator       = container_type::reverse_iterator;
using const_reverse_iterator = container_type::const_reverse_iterator;
```

## Member Functions

### Default Constructor

```cpp
preserve_comments() = default;
```

Constructs an empty `preserve_comments`.

### Copy and Move Constructors

```cpp
preserve_comments(preserve_comments const&) = default;
preserve_comments(preserve_comments &&)     = default;
```

Constructs a `preserve_comments` by copying or moving.

### Constructor (`std::vector<std::string>`)

```cpp
explicit preserve_comments(const std::vector<std::string>& c);
explicit preserve_comments(std::vector<std::string>&& c);
```

Constructs a `preserve_comments` holding the contents of `std::vector<std::string>`.

### Constructor (`discard_comments`)

```cpp
explicit preserve_comments(const discard_comments&);
```

Constructs an empty `preserve_comments`.

### Constructor (`Iterator`)

```cpp
template<typename InputIterator>
preserve_comments(InputIterator first, InputIterator last);
```

Constructs a `preserve_comments` from the range represented by `InputIterator` pointing to `std::string`.

### Constructor (`std::initializer_list`)

```cpp
preserve_comments(std::initializer_list<std::string> x);
```

Constructs a `preserve_comments` from the range represented by `std::initializer_list<std::string>`.

### Constructor (Size Specified)

```cpp
explicit preserve_comments(size_type n);
preserve_comments(size_type n, const std::string& x);
```

Constructs a `preserve_comments` with `n` comments.

If a `std::string` is passed, it replicates that comment `n` times.

### Destructor

```cpp
~preserve_comments() = default;
```

Destroys the `preserve_comments`.

### `operator=(preserve_comments)`

```cpp
preserve_comments& operator=(preserve_comments const&) = default;
preserve_comments& operator=(preserve_comments &&)     = default;
```

Assigns a `preserve_comments` by copying or moving.

### `operator=(std::vector<std::string>)`

```cpp
preserve_comments& operator=(const std::vector<std::string>& c);
preserve_comments& operator=(std::vector<std::string>&& c);
```

Assigns a `std::vector<std::string>` by copying or moving.

### `assign`

```cpp
template<typename InputIterator>
void assign(InputIterator first, InputIterator last);
void assign(std::initializer_list<std::string> ini);
void assign(size_type n, const std::string& val);
```

The same as `std::vector<std::string>::assign`.

### `insert`

```cpp
iterator insert(const_iterator p, const std::string& x);
iterator insert(const_iterator p, std::string&&      x);
iterator insert(const_iterator p, size_type n, const std::string& x);
template<typename InputIterator>
iterator insert(const_iterator p, InputIterator first, InputIterator last);
iterator insert(const_iterator p, std::initializer_list<std::string> ini);
```

The same as `std::vector<std::string>::insert`.

### `emplace`

```cpp
template<typename ... Ts>
iterator emplace(const_iterator p, Ts&& ... args);
```

The same as `std::vector<std::string>::emplace`.

### `erase`

```cpp
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
```

The same as `std::vector<std::string>::erase`.

### `swap`

```cpp
void swap(preserve_comments& other);
```

Swaps the comments.

### `push_back`

```cpp
void push_back(const std::string& v);
void push_back(std::string&&      v);
```

The same as `std::vector<std::string>::push_back`.

### `pop_back`

```cpp
void pop_back();
```

The same as `std::vector<std::string>::pop_back`.

### `emplace_back`

```cpp
template<typename ... Ts>
void emplace_back(Ts&& ... args);
```

The same as `std::vector<std::string>::emplace_back`.

### `clear`

```cpp
void clear();
```

The same as `std::vector<std::string>::clear`.

### `size`

```cpp
size_type size() const noexcept;
```

The same as `std::vector<std::string>::size`.

### `max_size`

```cpp
size_type max_size() const noexcept;
```

The same as `std::vector<std::string>::max_size`.

### `capacity`

```cpp
size_type capacity() const noexcept;
```

The same as `std::vector<std::string>::capacity`.

### `empty`

```cpp
bool empty() const noexcept;
```

The same as `std::vector<std::string>::empty`.

### `reserve`

```cpp
void reserve(size_type n);
```

The same as `std::vector<std::string>::reserve`.

### `resize`

```cpp
void resize(size_type n);
void resize(size_type n, const std::string& c);
```

The same as `std::vector<std::string>::resize`.

### `shrink_to_fit`

```cpp
void shrink_to_fit();
```

The same as `std::vector<std::string>::shrink_to_fit`.

### `operator[]`

```cpp
reference       operator[](const size_type n)       noexcept;
const_reference operator[](const size_type n) const noexcept;
```

The same as `std::vector<std::string>::operator[]`.

### `at`

```cpp
reference       at(const size_type n)      ;
const_reference at(const size_type n) const;
```

The same as `std::vector<std::string>::at`.

### `front`

```cpp
reference       front()       noexcept;
const_reference front() const noexcept;
```

The same as `std::vector<std::string>::front`.

### `back`

```cpp
reference       back()        noexcept;
const_reference back()  const noexcept;
```

The same as `std::vector<std::string>::back`.

### `data`

```cpp
pointer         data()        noexcept;
const_pointer   data()  const noexcept;
```

The same as `std::vector<std::string>::data`.

### `begin/end`

```cpp
iterator       begin()        noexcept;
iterator       end()          noexcept;
const_iterator begin()  const noexcept;
const_iterator end()    const noexcept;
const_iterator cbegin() const noexcept;
const_iterator cend()   const noexcept;
```

The same as `std::vector<std::string>::begin/end`.

### `rbegin/rend`

```cpp
reverse_iterator       rbegin()        noexcept;
reverse_iterator       rend()          noexcept;
const_reverse_iterator rbegin()  const noexcept;
const_reverse_iterator rend()    const noexcept;
const_reverse_iterator crbegin() const noexcept;
const_reverse_iterator crend()   const noexcept;
```

The same as `std::vector<std::string>::rbegin/rend`

## non-member functions

### Comparison operator

```cpp
bool operator==(const preserve_comments&, const preserve_comments&);
bool operator!=(const preserve_comments&, const preserve_comments&);
bool operator< (const preserve_comments&, const preserve_comments&);
bool operator<=(const preserve_comments&, const preserve_comments&);
bool operator> (const preserve_comments&, const preserve_comments&);
bool operator>=(const preserve_comments&, const preserve_comments&);
```

It compares two `preserve_comments` in the same way as `std::vector<std::string>`.

### `swap`

```cpp
void swap(preserve_comments&,        preserve_comments&);
void swap(preserve_comments&,        std::vector<std::string>&);
void swap(std::vector<std::string>&, preserve_comments&);
```

### Stream operator

```cpp
std::ostream& operator<<(std::ostream&, const preserve_comments&);
```

Outputs as TOML comments.

If the first character is not `#`, it adds `#`.

# `toml::discard_comments`

`discard_comments` serves as a container for discarding comments.

It encompasses all the member functions of `std::vector<std::string>`, but invoking functions that modify the content has no effect, leaving it perpetually empty.

```cpp
namespace toml
{
class discard_comments;

bool operator==(const discard_comments&, const discard_comments&);
bool operator!=(const discard_comments&, const discard_comments&);
bool operator< (const discard_comments&, const discard_comments&);
bool operator<=(const discard_comments&, const discard_comments&);
bool operator> (const discard_comments&, const discard_comments&);
bool operator>=(const discard_comments&, const discard_comments&);

void swap(discard_comments&, discard_comments&);

std::ostream& operator<<(std::ostream&, const discard_comments&);
} //toml
```

## Member types

```cpp
// container_type is not defined
using size_type              = std::size_t;
using difference_type        = std::ptrdiff_t;
using value_type             = std::string;
using reference              = std::string&;
using const_reference        = std::string const&;
using pointer                = std::string*;
using const_pointer          = std::string const*;
using iterator               = /* internal type: empty-iterator */
using const_iterator         = /* internal type: empty-iterator */
using reverse_iterator       = /* internal type: empty-iterator */
using const_reverse_iterator = /* internal type: empty-iterator */
```

### Default Constructor

```cpp
discard_comments() = default;
```

Constructs an empty `discard_comments`.

### Copy / Move Constructor

```cpp
discard_comments(discard_comments const&) = default;
discard_comments(discard_comments &&)     = default;
```

Constructs a `discard_comments` by copying or moving.

### Constructor(`std::vector<std::string>`)

```cpp
explicit discard_comments(const std::vector<std::string>& c);
explicit discard_comments(std::vector<std::string>&& c);
```

Constructs an empty `discard_comments`. Arguments are ignored.

### Constructor(`preserve_comments`)

```cpp
explicit discard_comments(const preserve_comments&);
```

Constructs an empty `discard_comments`. Arguments are ignored.

### Constructor(`Iterator`)

```cpp
template<typename InputIterator>
discard_comments(InputIterator first, InputIterator last);
```

Constructs an empty `discard_comments`. Arguments are ignored.

### Constructor(`std::initializer_list`)

```cpp
discard_comments(std::initializer_list<std::string> x);
```

Constructs an empty `discard_comments`. Arguments are ignored.

### Constructor(サイズ指定)

```cpp
explicit discard_comments(size_type n);
discard_comments(size_type n, const std::string& x);
```

Constructs an empty `discard_comments`. Arguments are ignored.

### Destructor

```cpp
~discard_comments() = default;
```

Destroys `discard_comments`.

### `operator=(discard_comments)`

```cpp
discard_comments& operator=(discard_comments const&) = default;
discard_comments& operator=(discard_comments &&)     = default;
```

Assigns `discard_comments` by copying or moving.

### `operator=(std::vector<std::string>)`

```cpp
discard_comments& operator=(const std::vector<std::string>& c);
discard_comments& operator=(std::vector<std::string>&& c);
```

Does nothing.

### `assign`

```cpp
template<typename InputIterator>
void assign(InputIterator first, InputIterator last);
void assign(std::initializer_list<std::string> ini);
void assign(size_type n, const std::string& val);
```

Does nothing.

### `insert`

```cpp
iterator insert(const_iterator p, const std::string& x);
iterator insert(const_iterator p, std::string&&      x);
iterator insert(const_iterator p, size_type n, const std::string& x);
template<typename InputIterator>
iterator insert(const_iterator p, InputIterator first, InputIterator last);
iterator insert(const_iterator p, std::initializer_list<std::string> ini);
```

Does nothing.

### `emplace`

```cpp
template<typename ... Ts>
iterator emplace(const_iterator p, Ts&& ... args);
```

Does nothing.

### `erase`

```cpp
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
```

Does nothing.

### `swap`

```cpp
void swap(discard_comments& other);
```

Swaps two `discard_comments`. Effectively it does nothing.

### `push_back`

```cpp
void push_back(const std::string& v);
void push_back(std::string&&      v);
```

Does nothing.

### `pop_back`

```cpp
void pop_back();
```

Does nothing.

### `emplace_back`

```cpp
template<typename ... Ts>
void emplace_back(Ts&& ... args);
```

Does nothing.

### `clear`

```cpp
void clear();
```

Does nothing.

### `size`

```cpp
size_type size() const noexcept;
```

Returns `0`.

### `max_size`

```cpp
size_type max_size() const noexcept;
```

Returns `0`.

### `capacity`

```cpp
size_type capacity() const noexcept;
```

Returns `0`.

### `empty`

```cpp
bool empty() const noexcept;
```

Returns `true`.

### `reserve`

```cpp
void reserve(size_type n);
```

Does nothing.

### `resize`

```cpp
void resize(size_type n);
void resize(size_type n, const std::string& c);
```

Does nothing.

### `shrink_to_fit`

```cpp
void shrink_to_fit();
```

Does nothing.

### `operator[]`

```cpp
reference       operator[](const size_type n)       noexcept;
const_reference operator[](const size_type n) const noexcept;
```

Causes undefined behavior.

### `at`

```cpp
reference       at(const size_type n)      ;
const_reference at(const size_type n) const;
```

Throws `std::out_of_range`.

### `front`

```cpp
reference       front()       noexcept;
const_reference front() const noexcept;
```

Causes undefined behavior.

### `back`

```cpp
reference       back()        noexcept;
const_reference back()  const noexcept;
```

Causes undefined behavior.

### `data`

```cpp
pointer         data()        noexcept;
const_pointer   data()  const noexcept;
```

Returns `nullptr`.

### `begin/end`

```cpp
iterator       begin()        noexcept;
iterator       end()          noexcept;
const_iterator begin()  const noexcept;
const_iterator end()    const noexcept;
const_iterator cbegin() const noexcept;
const_iterator cend()   const noexcept;
```

Returns an internally defined `empty-iterator`.

The `empty-iterator` remains at the same value after incrementing or decrementing and all values are equivalent.

Accessing the target of the `empty-iterator` always calls `std::terminate`.


### `rbegin/rend`

```cpp
reverse_iterator       rbegin()        noexcept;
reverse_iterator       rend()          noexcept;
const_reverse_iterator rbegin()  const noexcept;
const_reverse_iterator rend()    const noexcept;
const_reverse_iterator crbegin() const noexcept;
const_reverse_iterator crend()   const noexcept;
```

Returns an internally defined `empty-iterator`.

The `empty-iterator` remains at the same value after incrementing or decrementing and all values are equivalent.

Accessing the target of the `empty-iterator` always calls `std::terminate`.

## non-member functions

### Comparison operator

```cpp
bool operator==(const discard_comments&, const discard_comments&);
bool operator!=(const discard_comments&, const discard_comments&);
bool operator< (const discard_comments&, const discard_comments&);
bool operator<=(const discard_comments&, const discard_comments&);
bool operator> (const discard_comments&, const discard_comments&);
bool operator>=(const discard_comments&, const discard_comments&);
```

All the instances of `discard_comments` are the same value. `operator==` returns `true`, `operator=!` returns `false`.

### `swap`

```cpp
void swap(discard_comments&, discard_comments&);
```

Swaps `discard_comments`. Effectively it does nothing.

### Stream operator

```cpp
std::ostream& operator<<(std::ostream&, const discard_comments&);
```

Outputs nothing.

# Related

- [value.hpp]({{<ref "value.md">}})

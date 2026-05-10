+++
title = "ordered_map.hpp"
type  = "docs"
+++

# ordered_map.hpp

Defines `toml::ordered_map`, which is used to maintain the order of values in a file.

# `class ordered_map`

```cpp
namespace toml
{
template<typename Key, typename Val, typename Cmp = std::equal_to<Key>,
         typename Allocator = std::allocator<std::pair<Key, Val>>>
class ordered_map;
}
```

The `ordered_map` is a `map` type that preserves the insertion order of values, allowing iteration in that order.

As a linear container, searches require `O(n)` time relative to the number of elements.
Use this when search operations are infrequent and maintaining the order of values is important.

## Non-Member Types

```cpp
namespace toml
{
struct ordered_type_config;

using ordered_value = basic_value<ordered_type_config>;
using ordered_table = typename ordered_value::table_type;
using ordered_array = typename ordered_value::array_type;
}
```

Use these in place of `toml::type_config` and `toml::value`.

{{< hint info >}}

Since `toml::parse` defaults to using `type_config`, specify

```cpp
const auto input = toml::parse<toml::ordered_type_config>("input.toml");
```

when parsing.

{{< /hint >}}

## Member Types

```cpp
using key_type       = Key;
using mapped_type    = Val;
using value_type     = std::pair<Key, Val>;
using key_compare    = Cmp;
using allocator_type = Allocator;

using container_type  = std::vector<value_type, Allocator>;
using reference       = typename container_type::reference;
using pointer         = typename container_type::pointer;
using const_reference = typename container_type::const_reference;
using const_pointer   = typename container_type::const_pointer;
using iterator        = typename container_type::iterator;
using const_iterator  = typename container_type::const_iterator;
using size_type       = typename container_type::size_type;
using difference_type = typename container_type::difference_type;
```

## Member Functions

### Constructors

```cpp
ordered_map() = default;
```

Constructs an empty `ordered_map`.

### Constructors (Comparator, Allocator)

```cpp
explicit ordered_map(const Cmp& cmp, const Allocator& alloc = Allocator());
explicit ordered_map(const Allocator& alloc);
```

Constructs an `ordered_map` with a specified comparator for key comparison and an allocator for memory management.

### Copy and Move Constructors

```cpp
ordered_map(const ordered_map&) = default;
ordered_map(ordered_map&&)      = default;

ordered_map(const ordered_map& other, const Allocator& alloc);
ordered_map(ordered_map&& other, const Allocator& alloc);
```

Constructs an `ordered_map` by copying or moving the contents from another `ordered_map`.
An allocator can also be specified for memory management.

### Constructors (Iterator)

```cpp
template<typename InputIterator>
ordered_map(InputIterator first, InputIterator last, const Cmp& cmp = Cmp(), const Allocator& alloc = Allocator());
template<typename InputIterator>
ordered_map(InputIterator first, InputIterator last, const Allocator& alloc = Allocator());
```

Constructs an `ordered_map` with a range represented by iterators.
The order of the elements follows the order of the iterators.

### Constructors (std::initializer_list)

```cpp
ordered_map(std::initializer_list<value_type> v, const Cmp& cmp = Cmp(), const Allocator& alloc = Allocator());
ordered_map(std::initializer_list<value_type> v, const Allocator& alloc);
```

Initializes the `ordered_map` using an initializer list.

### Copy and Move Assignment Operators

```cpp
ordered_map& operator=(const ordered_map&) = default;
ordered_map& operator=(ordered_map&&)      = default;
```

Assigns the contents of another `ordered_map` to this one, using copy or move semantics.

### Assignment Operator (std::initializer_list)

```cpp
ordered_map& operator=(std::initializer_list<value_type> v);
```

Assigns the contents of an initializer list to the `ordered_map`.

### Destructor

```cpp
~ordered_map() = default;
```

Destroys the `ordered_map`.

### `begin()`, `end()`

```cpp
iterator       begin()        noexcept;
iterator       end()          noexcept;
const_iterator begin()  const noexcept;
const_iterator end()    const noexcept;
const_iterator cbegin() const noexcept;
const_iterator cend()   const noexcept;
```

Returns iterators to the beginning and end of the container, allowing iteration over its contents in order.


### `empty()`

```cpp
bool empty() const noexcept;
```

Returns `true` if the `ordered_map` is empty, `false` otherwise.

### `size()`

```cpp
std::size_t size() const noexcept;
```

Returns the number of elements in the `ordered_map`.

### `max_size()`

```cpp
std::size_t max_size() const noexcept;
```

Returns the maximum number of elements the `ordered_map` can hold.

### `clear()`

```cpp
void clear();
```

Clears all elements from the `ordered_map`.

### `push_back(kv)`

```cpp
void push_back(const value_type&);
void push_back(value_type&&);
```

Appends a key-value pair to the end of the `ordered_map`.

### `emplace_back(k, v)`

```cpp
void emplace_back(key_type, mapped_type);
```

Appends a key-value pair to the end of the `ordered_map` by constructing it in place.

### `pop_back()`

```cpp
void pop_back();
```

Removes the last element from the `ordered_map`.

### `insert(kv)`

```cpp
void insert(value_type);
```

Inserts a key-value pair at the end of the `ordered_map`.

### `emplace(k, v)`

```cpp
void emplace(key_type, mapped_type);
```

Inserts a key-value pair at the end of the `ordered_map` by constructing it in place.

### `count(k)`

```cpp
std::size_t count(const key_type&) const noexcept;
```

Returns the number of elements with the specified key.
Since duplicate keys are not allowed, this will return either `1` if the key exists or `0` if it does not.

### `contains(k)`

```cpp
bool contains(const key_type&) const noexcept;
```

Returns `true` if the `ordered_map` contains an element with the specified key, `false` otherwise.

### `find(k)`

```cpp
iterator       find(const key_type& key)       noexcept;
const_iterator find(const key_type& key) const noexcept;
```

Finds an element with the specified key and returns an iterator to it. If the key is not found, returns `end()`.

### `at(k)`

```cpp
mapped_type&       at(const key_type& k);
mapped_type const& at(const key_type& k) const;
```

Finds an element with the specified key and returns a reference to its value. Throws `std::out_of_range` if the key is not found.

### `operator[](k)`

```cpp
mapped_type&       operator[](const key_type& k);
mapped_type const& operator[](const key_type& k) const;
```

Finds an element with the specified key and returns a reference to its value.
If the key is not found, a new value is constructed and returned.
If the `ordered_map` is `const`, throws `std::out_of_range` instead.

### `erase(...)`

```cpp
iterator erase(iterator pos);
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
size_type erase(const key_type& key);
```

Removes values pointed by an iterator or a value corresponding to a key.

### `key_comp()`

```cpp
key_compare key_comp() const;
```

Returns the comparator used for key comparison.

## Notes

### Key Modification

{{< hint warning >}}

Since `ordered_map` uses `std::pair<Key, Val>` for `value_type`, it is possible to modify the key through an iterator. However, this practice is not recommended.

If you modify a key this way and it conflicts with an existing key, one of the conflicting keys will become unsearchable.

When using `operator[]`, `push_back`, or `insert`, collisions with existing keys are detected.

{{< /hint >}}

### Order Preservation Details

{{< hint warning >}}

`ordered_map` maintains the order of keys, but this order preservation applies only to keys defined within the same table. Order across different tables is not maintained.

For example, the order in the following file will be preserved:

```cpp
apple.type = "fruit"
apple.skin = "thin"
apple.color = "red"
orange.type = "fruit"
orange.skin = "thick"
orange.color = "orange"
```

In contrast, the order in the following file will not be preserved:

```cpp
apple.type = "fruit"
orange.type = "fruit"

apple.skin = "thin"
orange.skin = "thick"

apple.color = "red"
orange.color = "orange"
```

`ordered_map` preserves the order of the `apple` and `orange` definitions at the root table level, and the order of `type`, `skin`, `color` within each `apple` and `orange` table.

{{< /hint >}}

## Related

- [parser.hpp]({{<ref "parser.md">}})
- [types.hpp]({{<ref "types.md">}})
- [value.hpp]({{<ref "value.md">}})

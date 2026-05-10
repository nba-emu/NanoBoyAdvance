+++
title = "ordered_map.hpp"
type  = "docs"
+++

# ordered_map.hpp

ファイル中の値の順番を維持するために使用する`toml::ordered_map`を定義します。

# `class ordered_map`

```cpp
namespace toml
{
template<typename Key, typename Val, typename Cmp = std::equal_to<Key>,
         typename Allocator = std::allocator<std::pair<Key, Val>>>
class ordered_map;
}
```

`ordered_map`は、値を追加した順序を保ったまま値を保持し、その順でイテレートできる `map` 型です。

線形コンテナなので、検索には要素数に対して `O(n)` の時間がかかります。
検索を行う機会が少なく、値の順序を守る必要がある場合に使用してください。


## 非メンバ型

```cpp
namespace toml
{
struct ordered_type_config;

using ordered_value = basic_value<ordered_type_config>;
using ordered_table = typename ordered_value::table_type;
using ordered_array = typename ordered_value::array_type;
}
```

`toml::type_config` と `toml::value` の代わりに使用します。

{{< hint info >}}

`toml::parse` はデフォルトで `type_config` を使用するので、パースする際に

```cpp
const auto input = toml::parse<toml::ordered_type_config>("input.toml");
```

としてください。

{{< /hint >}}

## メンバ型

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

## メンバ関数

### コンストラクタ

```cpp
ordered_map() = default;
```

空の `ordered_map` を構築します。

### コンストラクタ（コンパレータ、アロケータ）

```cpp
explicit ordered_map(const Cmp& cmp, const Allocator& alloc = Allocator());
explicit ordered_map(const Allocator& alloc);
```

キーの比較に使用するコンパレータや、コンテナのメモリ確保に使用するアロケータを指定して構築します。

### コピー・ムーブコンストラクタ

```cpp
ordered_map(const ordered_map&) = default;
ordered_map(ordered_map&&)      = default;

ordered_map(const ordered_map& other, const Allocator& alloc);
ordered_map(ordered_map&& other, const Allocator& alloc);
```

別の `ordered_map` の内容をコピー・ムーブして構築します。

コンテナのメモリ確保に使用するアロケータを指定することも可能です。

### コンストラクタ（`Iterator`）

```cpp
template<typename InputIterator>
ordered_map(InputIterator first, InputIterator last, const Cmp& cmp = Cmp(), const Allocator& alloc = Allocator());
template<typename InputIterator>
ordered_map(InputIterator first, InputIterator last, const Allocator& alloc = Allocator());
```

イテレータ範囲を受け取って構築します。

順序は、イテレータの順序に従います。

### コンストラクタ（`std::initializer_list`）

```cpp
ordered_map(std::initializer_list<value_type> v, const Cmp& cmp = Cmp(), const Allocator& alloc = Allocator());
ordered_map(std::initializer_list<value_type> v, const Allocator& alloc);
```

`ordered_map{...}`の形式で初期化します。

### コピー・ムーブ代入演算子

```cpp
ordered_map& operator=(const ordered_map&) = default;
ordered_map& operator=(ordered_map&&)      = default;
```

別の `ordered_map` の内容をコピー・ムーブ代入します。

### 代入演算子（`std::initializer_list`）

```cpp
ordered_map& operator=(std::initializer_list<value_type> v);
```

`std::initializer_list` の内容を代入します。

### デストラクタ

```cpp
~ordered_map() = default;
```

`ordered_map` を破棄します。

### `begin()`, `end()`

```cpp
iterator       begin()        noexcept;
iterator       end()          noexcept;
const_iterator begin()  const noexcept;
const_iterator end()    const noexcept;
const_iterator cbegin() const noexcept;
const_iterator cend()   const noexcept;
```

コンテナの内容を順序通りにイテレートします。

### `empty()`

```cpp
bool empty() const noexcept;
```

`ordered_map` が空の場合 `true` を、そうでない場合 `false` を返します。

### `size()`

```cpp
std::size_t size() const noexcept;
```

`ordered_map` の要素数を返します。

### `max_size()`

```cpp
std::size_t max_size() const noexcept;
```

`ordered_map` が持つことのできる最大の要素数を返します。

### `clear()`

```cpp
void clear();
```

内容を消去します。

### `push_back(kv)`

```cpp
void push_back(const value_type&);
void push_back(value_type&&);
```

キーと値のペアを末尾に追加します。

### `emplace_back(k, v)`

```cpp
void emplace_back(key_type, mapped_type);
```

キーと値のペアを末尾に追加します。

### `pop_back()`

```cpp
void pop_back();
```

末尾の値を削除します。

### `insert(kv)`

```cpp
void insert(value_type);
```

キーと値のペアを末尾に追加します。

### `emplace(k, v)`

```cpp
void emplace(key_type, mapped_type);
```

キーと値のペアを末尾に追加します。

### `count(k)`

```cpp
std::size_t count(const key_type&) const noexcept;
```

キーに対応する値の数を返します。

同じキーに複数の値を代入することはできないので、値が存在する場合は `1`, そうでない場合は `0` を返します。

### `contains(k)`

```cpp
bool contains(const key_type&) const noexcept;
```

キーに対応する値が存在するなら `true` を、そうでないなら `false` を返します。

### `find(k)`

```cpp
iterator       find(const key_type& key)       noexcept;
const_iterator find(const key_type& key) const noexcept;
```

キーに対応する値を検索し、それを指すイテレータを返します。

存在しなかった場合、`end()`を返します。

### `at(k)`

```cpp
mapped_type&       at(const key_type& k);
mapped_type const& at(const key_type& k) const;
```

キーに対応する値を検索して返します。

存在しなかった場合、`std::out_of_range`を送出します。

### `operator[](k)`

```cpp
mapped_type&       operator[](const key_type& k);
mapped_type const& operator[](const key_type& k) const;
```

キーに対応する値を検索して返します。

存在しなかった場合、新規な値を構築して返します。

`ordered_map` が `const` の場合は新規な値を構築できないので、 `std::out_of_range` を送出します。

### `erase(...)`

```cpp
iterator erase(iterator pos);
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
size_type erase(const key_type& key);
```

イテレータの指す位置の値、あるいはキーに該当する値を削除します。

### `key_comp()`

```cpp
key_compare key_comp() const;
```

比較に使用するコンパレータを返します。


## 使用上の注意

### キーの書き換え

{{< hint warning >}}

`ordered_map` は `value_type` に `std::pair<Key, Val>` を使用しているので、イテレータを介してキーを書き換えることが可能になってしまっています。

この方法でキーを書き換えることは推奨されません。

キーを書き換えて既存のキーと衝突した場合、衝突したうちの片方が検索できなくなります。

`operator[]` や `push_back`, `insert` による書き込みの場合は、既存のキーとの衝突が検出されます。

{{< /hint >}}

### 保たれる順序の詳細

{{< hint warning >}}

`ordered_map` はキーの順序を保ちますが、ここで保たれる順序は同じテーブルで定義されたキーの順序のみです。
よって、テーブルをまたいだ順序は保たれないことに注意してください。

例えば、以下のファイルの順序は保たれます。

```cpp
apple.type = "fruit"
apple.skin = "thin"
apple.color = "red"
orange.type = "fruit"
orange.skin = "thick"
orange.color = "orange"
```

対して以下のファイルの順序は保たれません。

```cpp
apple.type = "fruit"
orange.type = "fruit"

apple.skin = "thin"
orange.skin = "thick"

apple.color = "red"
orange.color = "orange"
```

`ordered_map` が保つ順序は、rootテーブルに定義された `apple` と `orange` の順序と、
`apple` 内で定義された `type`, `skin`, `color` の順序、
また `orange` 内で定義された `type`, `skin`, `color` の順序のみです。

{{< /hint >}}

## 関連項目

- [parser.hpp]({{<ref "parser.md">}})
- [types.hpp]({{<ref "types.md">}})
- [value.hpp]({{<ref "value.md">}})

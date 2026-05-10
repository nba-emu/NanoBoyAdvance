+++
title = "comments.hpp"
type  = "docs"
+++

# comments.hpp

`color.hpp`では、コメントを保持するクラスが提供されます。

# `toml::preserve_comments`

`preserve_comments`は、コメントを保持するコンテナです。

`std::vector<std::string>`が持つメンバ関数を全て持っています。

コメントは`std::string`として保持されます。
先頭が`#`でない場合、出力時に`#`が補われます。コンテナに要素として追加する段階では補われません。
スペースは補われないため、`#`の直後にスペースを入れたい場合、コメントをスペースから始めるか、`#`を含めたコメントを渡す必要があります。

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

## メンバ型

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

## メンバ関数

### デフォルトコンストラクタ

```cpp
preserve_comments() = default;
```

空の`preserve_comments`を構築します。

### コピー・ムーブコンストラクタ

```cpp
preserve_comments(preserve_comments const&) = default;
preserve_comments(preserve_comments &&)     = default;
```

`preserve_comments`をコピー・ムーブ構築します。

### コンストラクタ(`std::vector<std::string>`)

```cpp
explicit preserve_comments(const std::vector<std::string>& c);
explicit preserve_comments(std::vector<std::string>&& c);
```

`std::vector<std::string>`の内容を持つ`preserve_comments`を構築します。


### コンストラクタ(`discard_comments`)

```cpp
explicit preserve_comments(const discard_comments&);
```

空の`preserve_comments`を構築します。

### コンストラクタ(`Iterator`)

```cpp
template<typename InputIterator>
preserve_comments(InputIterator first, InputIterator last);
```

`std::string`を指す`InputIterator`が表す範囲から`preserve_comments`を構築します。

### コンストラクタ(`std::initializer_list`)

```cpp
preserve_comments(std::initializer_list<std::string> x);
```

`std::initializer_list<std::string>`が表す範囲から`preserve_comments`を構築します。

### コンストラクタ(サイズ指定)

```cpp
explicit preserve_comments(size_type n);
preserve_comments(size_type n, const std::string& x);
```

`n`個のコメントを持つ`preserve_comments`を構築します。

`std::string`を渡した場合、そのコメントを`n`個に複製します。

### デストラクタ

```cpp
~preserve_comments() = default;
```

`preserve_comments`を破棄します。

### `operator=(preserve_comments)`

```cpp
preserve_comments& operator=(preserve_comments const&) = default;
preserve_comments& operator=(preserve_comments &&)     = default;
```

`preserve_comments`をコピー・ムーブ代入します。

### `operator=(std::vector<std::string>)`

```cpp
preserve_comments& operator=(const std::vector<std::string>& c);
preserve_comments& operator=(std::vector<std::string>&& c);
```

`std::vector<std::string>`をコピー・ムーブ代入します。

### `assign`

```cpp
template<typename InputIterator>
void assign(InputIterator first, InputIterator last);
void assign(std::initializer_list<std::string> ini);
void assign(size_type n, const std::string& val);
```

`std::vector<std::string>::assign`と同等の効果を持ちます。

### `insert`

```cpp
iterator insert(const_iterator p, const std::string& x);
iterator insert(const_iterator p, std::string&&      x);
iterator insert(const_iterator p, size_type n, const std::string& x);
template<typename InputIterator>
iterator insert(const_iterator p, InputIterator first, InputIterator last);
iterator insert(const_iterator p, std::initializer_list<std::string> ini);
```

`std::vector<std::string>::insert`と同等の効果を持ちます。

### `emplace`

```cpp
template<typename ... Ts>
iterator emplace(const_iterator p, Ts&& ... args);
```

`std::vector<std::string>::insert`と同等の効果を持ちます。

### `erase`

```cpp
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
```

`std::vector<std::string>::insert`と同等の効果を持ちます。

### `swap`

```cpp
void swap(preserve_comments& other);
```

他の`preserve_comments`と内容を交換します。

### `push_back`

```cpp
void push_back(const std::string& v);
void push_back(std::string&&      v);
```

`std::vector<std::string>::push_back`と同等の効果を持ちます。

### `pop_back`

```cpp
void pop_back();
```

`std::vector<std::string>::pop_back`と同等の効果を持ちます。

### `emplace_back`

```cpp
template<typename ... Ts>
void emplace_back(Ts&& ... args);
```

`std::vector<std::string>::emplace_back`と同等の効果を持ちます。

### `clear`

```cpp
void clear();
```

`std::vector<std::string>::clear`と同等の効果を持ちます。

### `size`

```cpp
size_type size() const noexcept;
```

`std::vector<std::string>::size`と同等の効果を持ちます。

### `max_size`

```cpp
size_type max_size() const noexcept;
```

`std::vector<std::string>::max_size`と同等の効果を持ちます。

### `capacity`

```cpp
size_type capacity() const noexcept;
```

`std::vector<std::string>::capacity`と同等の効果を持ちます。

### `empty`

```cpp
bool empty() const noexcept;
```

`std::vector<std::string>::empty`と同等の効果を持ちます。

### `reserve`

```cpp
void reserve(size_type n);
```

`std::vector<std::string>::reserve`と同等の効果を持ちます。

### `resize`

```cpp
void resize(size_type n);
void resize(size_type n, const std::string& c);
```

`std::vector<std::string>::resize`と同等の効果を持ちます。

### `shrink_to_fit`

```cpp
void shrink_to_fit();
```

`std::vector<std::string>::shrink_to_fit`と同等の効果を持ちます。

### `operator[]`

```cpp
reference       operator[](const size_type n)       noexcept;
const_reference operator[](const size_type n) const noexcept;
```

`std::vector<std::string>::operator[]`と同等の効果を持ちます。

### `at`

```cpp
reference       at(const size_type n)      ;
const_reference at(const size_type n) const;
```

`std::vector<std::string>::at`と同等の効果を持ちます。

### `front`

```cpp
reference       front()       noexcept;
const_reference front() const noexcept;
```

`std::vector<std::string>::front`と同等の効果を持ちます。

### `back`

```cpp
reference       back()        noexcept;
const_reference back()  const noexcept;
```

`std::vector<std::string>::back`と同等の効果を持ちます。

### `data`

```cpp
pointer         data()        noexcept;
const_pointer   data()  const noexcept;
```

`std::vector<std::string>::data`と同等の効果を持ちます。

### `begin/end`

```cpp
iterator       begin()        noexcept;
iterator       end()          noexcept;
const_iterator begin()  const noexcept;
const_iterator end()    const noexcept;
const_iterator cbegin() const noexcept;
const_iterator cend()   const noexcept;
```

`std::vector<std::string>::begin/end`と同等の効果を持ちます。

### `rbegin/rend`

```cpp
reverse_iterator       rbegin()        noexcept;
reverse_iterator       rend()          noexcept;
const_reverse_iterator rbegin()  const noexcept;
const_reverse_iterator rend()    const noexcept;
const_reverse_iterator crbegin() const noexcept;
const_reverse_iterator crend()   const noexcept;
```

`std::vector<std::string>::rbegin/rend`と同等の効果を持ちます。

## 非メンバ関数

### 比較演算子

```cpp
bool operator==(const preserve_comments&, const preserve_comments&);
bool operator!=(const preserve_comments&, const preserve_comments&);
bool operator< (const preserve_comments&, const preserve_comments&);
bool operator<=(const preserve_comments&, const preserve_comments&);
bool operator> (const preserve_comments&, const preserve_comments&);
bool operator>=(const preserve_comments&, const preserve_comments&);
```

`std::vector<std::string>`と同様に比較を行います。

### `swap`

```cpp
void swap(preserve_comments&,        preserve_comments&);
void swap(preserve_comments&,        std::vector<std::string>&);
void swap(std::vector<std::string>&, preserve_comments&);
```

### ストリーム演算子

```cpp
std::ostream& operator<<(std::ostream&, const preserve_comments&);
```

コメントとして出力します。

先頭が`#`でない場合、`#`が補われます。

# `toml::discard_comments`

`discard_comments`は、コメントを破棄するコンテナです。

`std::vector<std::string>`が持つメンバ関数を全て持っていますが、内容を変更する関数を呼び出しても何も効果はなく、常に空になります。

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

## メンバ型

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

### デフォルトコンストラクタ

```cpp
discard_comments() = default;
```

空の`discard_comments`を構築します。

### コピー・ムーブコンストラクタ

```cpp
discard_comments(discard_comments const&) = default;
discard_comments(discard_comments &&)     = default;
```

`discard_comments`をコピー・ムーブ構築します。

### コンストラクタ(`std::vector<std::string>`)

```cpp
explicit discard_comments(const std::vector<std::string>& c);
explicit discard_comments(std::vector<std::string>&& c);
```

空の`discard_comments`を構築します。引数の内容は無視されます。

### コンストラクタ(`preserve_comments`)

```cpp
explicit discard_comments(const preserve_comments&);
```

空の`discard_comments`を構築します。引数の内容は無視されます。

### コンストラクタ(`Iterator`)

```cpp
template<typename InputIterator>
discard_comments(InputIterator first, InputIterator last);
```

空の`discard_comments`を構築します。引数の内容は無視されます。

### コンストラクタ(`std::initializer_list`)

```cpp
discard_comments(std::initializer_list<std::string> x);
```

空の`discard_comments`を構築します。引数の内容は無視されます。

### コンストラクタ(サイズ指定)

```cpp
explicit discard_comments(size_type n);
discard_comments(size_type n, const std::string& x);
```

空の`discard_comments`を構築します。引数の内容は無視されます。

### デストラクタ

```cpp
~discard_comments() = default;
```

`discard_comments`を破棄します。

### `operator=(discard_comments)`

```cpp
discard_comments& operator=(discard_comments const&) = default;
discard_comments& operator=(discard_comments &&)     = default;
```

`discard_comments`をコピー・ムーブ代入します。

### `operator=(std::vector<std::string>)`

```cpp
discard_comments& operator=(const std::vector<std::string>& c);
discard_comments& operator=(std::vector<std::string>&& c);
```

何もしません。引数の内容は無視されます。

### `assign`

```cpp
template<typename InputIterator>
void assign(InputIterator first, InputIterator last);
void assign(std::initializer_list<std::string> ini);
void assign(size_type n, const std::string& val);
```

何もしません。引数の内容は無視されます。

### `insert`

```cpp
iterator insert(const_iterator p, const std::string& x);
iterator insert(const_iterator p, std::string&&      x);
iterator insert(const_iterator p, size_type n, const std::string& x);
template<typename InputIterator>
iterator insert(const_iterator p, InputIterator first, InputIterator last);
iterator insert(const_iterator p, std::initializer_list<std::string> ini);
```

何もしません。引数の内容は無視されます。

### `emplace`

```cpp
template<typename ... Ts>
iterator emplace(const_iterator p, Ts&& ... args);
```

何もしません。引数の内容は無視されます。

### `erase`

```cpp
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
```

何もしません。引数の内容は無視されます。

### `swap`

```cpp
void swap(discard_comments& other);
```

他の`discard_comments`と内容を交換します。

### `push_back`

```cpp
void push_back(const std::string& v);
void push_back(std::string&&      v);
```

何もしません。引数の内容は無視されます。

### `pop_back`

```cpp
void pop_back();
```

何もしません。引数の内容は無視されます。

### `emplace_back`

```cpp
template<typename ... Ts>
void emplace_back(Ts&& ... args);
```

何もしません。引数の内容は無視されます。

### `clear`

```cpp
void clear();
```

何もしません。引数の内容は無視されます。

### `size`

```cpp
size_type size() const noexcept;
```

常に`0`を返します。

### `max_size`

```cpp
size_type max_size() const noexcept;
```

常に`0`を返します。

### `capacity`

```cpp
size_type capacity() const noexcept;
```

常に`0`を返します。

### `empty`

```cpp
bool empty() const noexcept;
```

常に`true`を返します。

### `reserve`

```cpp
void reserve(size_type n);
```

何もしません。引数の内容は無視されます。

### `resize`

```cpp
void resize(size_type n);
void resize(size_type n, const std::string& c);
```

何もしません。引数の内容は無視されます。

### `shrink_to_fit`

```cpp
void shrink_to_fit();
```

何もしません。引数の内容は無視されます。

### `operator[]`

```cpp
reference       operator[](const size_type n)       noexcept;
const_reference operator[](const size_type n) const noexcept;
```

未定義動作です。

### `at`

```cpp
reference       at(const size_type n)      ;
const_reference at(const size_type n) const;
```

`std::out_of_range`を送出します。

### `front`

```cpp
reference       front()       noexcept;
const_reference front() const noexcept;
```

未定義動作です。

### `back`

```cpp
reference       back()        noexcept;
const_reference back()  const noexcept;
```

未定義動作です。

### `data`

```cpp
pointer         data()        noexcept;
const_pointer   data()  const noexcept;
```

常に`nullptr`を返します。

### `begin/end`

```cpp
iterator       begin()        noexcept;
iterator       end()          noexcept;
const_iterator begin()  const noexcept;
const_iterator end()    const noexcept;
const_iterator cbegin() const noexcept;
const_iterator cend()   const noexcept;
```

内部で定義された`empty-iterator`を返します。

`empty-iterator`はインクリメントやデクリメントしても同じ値でとどまり、全ての値が同値です。

`empty-iterator`が指す先にアクセスすると常に`std::terminate`が呼び出されます。

### `rbegin/rend`

```cpp
reverse_iterator       rbegin()        noexcept;
reverse_iterator       rend()          noexcept;
const_reverse_iterator rbegin()  const noexcept;
const_reverse_iterator rend()    const noexcept;
const_reverse_iterator crbegin() const noexcept;
const_reverse_iterator crend()   const noexcept;
```

内部で定義された`empty-iterator`を返します。

`empty-iterator`はインクリメントやデクリメントしても同じ値でとどまり、全ての値が同値です。

`empty-iterator`が指す先にアクセスすると常に`std::terminate`が呼び出されます。

## 非メンバ関数

### 比較演算子

```cpp
bool operator==(const discard_comments&, const discard_comments&);
bool operator!=(const discard_comments&, const discard_comments&);
bool operator< (const discard_comments&, const discard_comments&);
bool operator<=(const discard_comments&, const discard_comments&);
bool operator> (const discard_comments&, const discard_comments&);
bool operator>=(const discard_comments&, const discard_comments&);
```

`discard_comments`は全て同じ値です。`==`には常に`true`を、`!=`には常に`false`を返します。

### `swap`

```cpp
void swap(discard_comments&, discard_comments&);
```

二つの`discard_comments`を交換します。

### ストリーム演算子

```cpp
std::ostream& operator<<(std::ostream&, const discard_comments&);
```

何も出力しません。

# 関連項目

- [value.hpp]({{<ref "value.md">}})

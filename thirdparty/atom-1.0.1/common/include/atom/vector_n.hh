// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <atom/integer.hh>
#include <atom/panic.hh>
#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>
#include <initializer_list>

#ifdef NDEBUG
  #define VECTOR_N_ASSERT_NOT_FULL()
  #define VECTOR_N_ASSERT_NOT_EMPTY()
  #define VECTOR_N_ASSERT_INDEX_IN_BOUNDS(index)
#else
  #define VECTOR_N_ASSERT_NOT_FULL() \
    if(!std::is_constant_evaluated() && Full()) { \
      ATOM_PANIC("{} called, however the vector was already full.", __PRETTY_FUNCTION__); \
    }

  #define VECTOR_N_ASSERT_NOT_EMPTY() \
    if(!std::is_constant_evaluated() && Empty()) { \
      ATOM_PANIC("{} called, however the vector was empty.", __PRETTY_FUNCTION__); \
    }

  #define VECTOR_N_ASSERT_INDEX_IN_BOUNDS(index) \
    if(!std::is_constant_evaluated() && index >= Size()) { \
      ATOM_PANIC("{} called with out-of-bounds index {} (size was {}).", __PRETTY_FUNCTION__, index, Size()); \
    }
#endif

namespace atom {

  template <typename T, std::size_t capacity>
  class Vector_N {
    public:
      using value_type = T;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using reference = T&;
      using const_reference = const T&;
      using pointer = T*;
      using const_pointer = const T*;

      using iterator = T*;
      using const_iterator = const T*;

      constexpr Vector_N() = default;

      constexpr Vector_N(std::initializer_list<T> values) {
        for(auto& value : values) PushBack(value);
      }

      template<class InputIt>
      constexpr Vector_N(InputIt first, InputIt last) {
        InputIt iterator = first;

        while(iterator != last) {
          PushBack(*iterator);
          ++iterator;
        }
      }

      constexpr ~Vector_N() {
        for(T& element : *this) element.~T();
      }

      constexpr T& operator[](std::size_t index) {
        VECTOR_N_ASSERT_INDEX_IN_BOUNDS(index);
        return Data()[index];
      }

      constexpr const T& operator[](std::size_t index) const {
        VECTOR_N_ASSERT_INDEX_IN_BOUNDS(index);
        return Data()[index];
      }

      constexpr void Clear() {
        m_size = 0;
      }

      constexpr void PushBack(T const& value) {
        VECTOR_N_ASSERT_NOT_FULL();
        Data()[m_size++] = value;
      }

      constexpr void PushBack(T&& value) {
        VECTOR_N_ASSERT_NOT_FULL();
        Data()[m_size++] = std::move(value);
      }

      template<typename... Args>
      constexpr void EmplaceBack(Args&&... args) {
        VECTOR_N_ASSERT_NOT_FULL();
        new(&Data()[m_size++]) T{std::forward<Args>(args)...};
      }

      constexpr void PopBack() {
        VECTOR_N_ASSERT_NOT_EMPTY();
        m_size--;
      }

      constexpr void Erase(const_iterator it) {
        VECTOR_N_ASSERT_NOT_EMPTY();

        auto copy_it = const_cast<iterator>(it);

        while (copy_it != end()) {
          *copy_it = std::move(*(copy_it + 1));
          ++copy_it;
        }

        m_size--;
      }

      constexpr iterator Insert(const_iterator it, T const& value) {
        VECTOR_N_ASSERT_NOT_FULL();

        iterator copy_it = end();

        while (copy_it != it) {
          *copy_it = std::move(*(copy_it - 1));
          --copy_it;
        }

        *const_cast<pointer>(it) = value;

        m_size++;

        return const_cast<iterator>(it);
      }

      constexpr iterator Insert(const_iterator it, T&& value) {
        VECTOR_N_ASSERT_NOT_FULL();

        iterator copy_it = end();

        while (copy_it != it) {
          *copy_it = std::move(*(copy_it - 1));
          --copy_it;
        }

        *const_cast<pointer>(it) = std::move(value);

        m_size++;

        return const_cast<iterator>(it);
      }

      constexpr reference Front() {
        return Data()[0];
      }

      constexpr const_reference Front() const {
        return Data()[0];
      }

      constexpr reference Back() {
        return Data()[m_size - 1];
      }

      constexpr const_reference Back() const {
        return Data()[m_size - 1];
      }

      [[nodiscard]] constexpr bool Empty() const {
        return m_size == 0u;
      }

      [[nodiscard]] constexpr bool Full() const {
        return m_size == capacity;
      }

      [[nodiscard]] constexpr size_t Size() const {
        return m_size;
      }

      constexpr pointer Data() {
        return reinterpret_cast<pointer>(m_buffer);
      }

      constexpr const_pointer Data() const {
        return reinterpret_cast<const_pointer>(m_buffer);
      }

      constexpr iterator begin() {
        return static_cast<iterator>(&Data()[0]);
      }

      constexpr iterator end() {
        return static_cast<iterator>(&Data()[m_size]);
      }

      constexpr const_iterator begin() const {
        return static_cast<const_iterator>(&Data()[0]);
      }

      constexpr const_iterator end() const {
        return static_cast<const_iterator>(&Data()[m_size]);
      }

      constexpr const_iterator cbegin() const {
        return static_cast<const_iterator>(&Data()[0]);
      }

      constexpr const_iterator cend() const {
        return static_cast<const_iterator>(&Data()[m_size]);
      }

      constexpr operator std::span<T>() {
        return std::span<T>{Data(), Size()};
      }

      [[nodiscard]] constexpr operator std::span<const T>() const {
        return std::span<const T>{Data(), Size()};
      }

    private:
      u8 m_buffer[sizeof(T) * capacity]{};
      size_t m_size{0u};
  };

} // namespace atom

#undef VECTOR_N_ASSERT_NOT_FULL
#undef VECTOR_N_ASSERT_NOT_EMPTY
#undef VECTOR_N_ASSERT_INDEX_IN_BOUNDS

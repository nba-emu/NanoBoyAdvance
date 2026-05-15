// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <atom/panic.hh>
#include <algorithm>
#include <memory>
#include <type_traits>

namespace atom {

  #define ATOM_RESULT_EMPTY (-1)
  #define ATOM_RESULT_SUCCESS 0

  #define ATOM_FORWARD_ERROR(result) do { \
    const auto status_code = (result).Code(); \
    if(status_code != ATOM_RESULT_SUCCESS) { \
      return status_code;\
    } \
  } while(0)

  namespace detail {

    // Storage<T> is a wrapper around the data we store.
    // It allows us to store references like Result<StatusCode, T&>
    // Move-semantics are implemented.
    // Copy-semantics are deleted. We never (accidentally) want to copy the underlying data.
    // Call Move() to move the underlying value out of the storage.
    template<typename T>
    struct Storage {
      Storage(const T& value) : m_value{value} {}
      Storage(T&& value) : m_value{std::move(value)} {}
      Storage(const Storage&) = delete;
      Storage(Storage&& other) noexcept { operator=(std::move(other)); }
      Storage& operator=(const Storage&) = delete;
      Storage& operator=(Storage&& other) noexcept {
        std::swap(m_value, other.m_value);
        return *this;
      }
      T&& Move() { return std::move(m_value); }
      T m_value;
    };

    template<typename T>
    struct Storage<T&> {
      Storage(T& value) : m_value{&value} {}
      Storage(const Storage&) = delete;
      Storage(Storage&& other) noexcept { operator=(std::move(other)); }
      Storage& operator=(const Storage&) = delete;
      Storage& operator=(Storage&& other) noexcept {
        std::swap(m_value, other.m_value);
        return *this;
      }
      T& Move() { return *m_value; }
      T* m_value{};
    };

  } // namespace detail

  template<typename StatusCode, typename T>
    requires std::is_move_constructible_v<T>
  class Result {
    public:
      Result(StatusCode status_code) : m_status_code{status_code} {}

      Result(const T& value) requires std::copy_constructible<T> : m_status_code{static_cast<StatusCode>(ATOM_RESULT_SUCCESS)}, m_value{value} {}

      Result(T&& value) : m_status_code{static_cast<StatusCode>(ATOM_RESULT_SUCCESS)}, m_value{std::move(value)} {}

      Result(const Result&) = delete;

      Result(Result&& other_result) {
        operator=(std::move(other_result));
      }

     ~Result() { // needed if `T` has a non-trivial destructor since m_value is in a union
        if(Ok()) {
          m_value.~Storage<T>();
        }
      }

      [[nodiscard]] bool Ok() const {
        return m_status_code == (StatusCode)ATOM_RESULT_SUCCESS;
      }

      [[nodiscard]] StatusCode Code() const {
        return m_status_code;
      }

      [[nodiscard]] T Unwrap() {
        if(!Ok()) {
          ATOM_PANIC("Attempted to unwrap an error result (status code: {})", (int)m_status_code);
        }
        m_status_code = static_cast<StatusCode>(ATOM_RESULT_EMPTY);
        return m_value.Move();
      }

      [[nodiscard]] T UnwrapOr(const T& fallback) requires std::is_copy_constructible_v<T> {
        if(!Ok()) {
          return fallback;
        }
        m_status_code = static_cast<StatusCode>(ATOM_RESULT_EMPTY);
        return m_value.Move();
      }

      Result& operator=(const Result&) = delete;

      Result& operator=(Result&& other_result) noexcept {
        std::swap(m_status_code, other_result.m_status_code);

        // As per C++11 standard a union member can only be initialized either during
        // initialization of the union or via placement-new. See: https://stackoverflow.com/a/33058919
        new (&m_value) detail::Storage<T>(std::move(other_result.m_value));
        return *this;
      }

    private:
      StatusCode m_status_code{};
      union { // Union used to avoid default initialization
        detail::Storage<T> m_value;
      };
  };

} // namespace atom

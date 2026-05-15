// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <type_traits>
#include <initializer_list>

namespace atom {

template<typename T>
  requires std::is_enum_v<T>
class BitFlags {
  public:
    using UnderlyingType = std::underlying_type_t<T>;

    constexpr BitFlags() = default;
    constexpr BitFlags(UnderlyingType value) : m_flags{value} {} // NOLINT(*-explicit-constructor)
    constexpr BitFlags(T flag) : m_flags{static_cast<UnderlyingType>(flag)} {} // NOLINT(*-explicit-constructor)

    constexpr BitFlags(std::initializer_list<T> flags) {
      for(T flag : flags) {
        m_flags |= static_cast<UnderlyingType>(flag);
      }
    }

    constexpr explicit operator bool() {
      return static_cast<bool>(m_flags);
    }

    constexpr bool operator==(BitFlags rhs) const { return m_flags == rhs.m_flags; }
    constexpr bool operator==(T rhs) const { return m_flags == static_cast<UnderlyingType>(rhs); }

    [[nodiscard]] constexpr BitFlags operator&(BitFlags rhs) const { return m_flags & rhs.m_flags; }
    [[nodiscard]] constexpr BitFlags operator|(BitFlags rhs) const { return m_flags | rhs.m_flags; }
    [[nodiscard]] constexpr BitFlags operator^(BitFlags rhs) const { return m_flags ^ rhs.m_flags; }

    constexpr BitFlags& operator&=(BitFlags rhs) { m_flags &= rhs.m_flags; return *this; }
    constexpr BitFlags& operator|=(BitFlags rhs) { m_flags |= rhs.m_flags; return *this; }
    constexpr BitFlags& operator^=(BitFlags rhs) { m_flags ^= rhs.m_flags; return *this; }

    constexpr BitFlags operator~() const { return ~m_flags; }

    [[nodiscard]] constexpr bool Has(T flag) const { return m_flags & static_cast<UnderlyingType>(flag); }
    [[nodiscard]] constexpr bool HasAny(BitFlags flags) { return m_flags & flags.m_flags; }
    [[nodiscard]] constexpr bool HasAll(BitFlags flags) { return (~m_flags & flags.m_flags) == 0; }
    [[nodiscard]] constexpr bool Empty() const { return !m_flags; }

    [[nodiscard]] constexpr UnderlyingType Raw() const { return m_flags; }

  private:
    UnderlyingType m_flags{};
};

#define BITFLAG_OPS(T) \
  [[nodiscard]] constexpr ::atom::BitFlags<T> operator|(T lhs, T rhs) {\
    return ::atom::BitFlags<T>{lhs} | ::atom::BitFlags<T>{rhs};\
  }\
  \
  [[nodiscard]] constexpr ::atom::BitFlags<T> operator~(T flag) {\
    return ~::atom::BitFlags<T>{flag};\
  }

} // namespace atom

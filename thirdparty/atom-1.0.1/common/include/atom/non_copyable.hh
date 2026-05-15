// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

namespace atom {

  class NonCopyable {
    public:
      NonCopyable(const NonCopyable&) = delete;
      NonCopyable& operator=(const NonCopyable&) = delete;

    protected:
      NonCopyable() = default;
     ~NonCopyable() = default;
  };

} // namespace atom

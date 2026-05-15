// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <fmt/format.h>
#include <string>
#include <utility>

namespace atom {

  namespace detail {

    [[noreturn]] void call_panic_handler(const char* file, int line, const char* message);

    template<typename... Args>
    [[noreturn]] void panic(const char* file, int line, const char* format, Args&&... args) {
      std::string message = fmt::format(fmt::runtime(format), std::forward<Args>(args)...);

      call_panic_handler(file, line, message.c_str());
    }

  } // namespace atom::detail

  typedef void (*PanicHandlerFn)(const char* file, int line, const char* message);

  void set_panic_handler(PanicHandlerFn handler);

} // namespace atom

#define ATOM_PANIC(format, ...) atom::detail::panic(__FILE__, __LINE__, format, ## __VA_ARGS__);

#define ATOM_ASSERT(condition, format, ...) if(!(condition)) ATOM_PANIC(format, ## __VA_ARGS__);

#define ATOM_UNREACHABLE() ATOM_PANIC("Reached supposedly unreachable code")
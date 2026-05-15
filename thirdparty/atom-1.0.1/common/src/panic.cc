// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#include <atom/panic.hh>
#include <cstdlib>
#include <fmt/color.h>

namespace atom {

  void default_panic_handler(const char* file, int line, const char* message) {
    fmt::print(fmt::fg(fmt::color::red), "panic: {}:{}: {}", file, line, message);
  }

  static PanicHandlerFn g_panic_handler_fn = &default_panic_handler;

  namespace detail {

    [[noreturn]] void call_panic_handler(const char* file, int line, const char* message) {
      g_panic_handler_fn(file, line, message);
      std::exit(-1);
    }

  } // namespace atom::detail

  void set_panic_handler(PanicHandlerFn handler) {
    g_panic_handler_fn = handler;
  }

} // namespace atom

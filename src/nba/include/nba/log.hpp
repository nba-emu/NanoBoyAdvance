/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>
#include <fmt/color.h>
#include <fmt/format.h>
#include <string_view>
#include <utility>

namespace nba {

enum Level {
  Trace =  1,
  Debug =  2,
  Info  =  4,
  Warn  =  8,
  Error = 16,
  Fatal = 32,

  All = Trace | Debug | Info | Warn | Error | Fatal
};

namespace detail {

#if defined(NDEBUG)
  static constexpr int kLogMask = Info | Warn | Error | Fatal;
#else
  static constexpr int kLogMask = All;
#endif

} // namespace nba::detail

template<Level level, typename... Args>
inline void Log(std::string_view format, Args&&... args) {
  if constexpr((detail::kLogMask & level) != 0) {
    fmt::text_style style = {};
    char const* prefix = "[?]";

    if constexpr(level == Trace) {
      style = fmt::fg(fmt::terminal_color::cyan);
      prefix = "[T]";
    }
    if constexpr(level == Debug) {
      style = fmt::fg(fmt::terminal_color::blue);
      prefix = "[D]";
    }
    if constexpr(level ==  Info) {
      prefix = "[I]";
    }
    if constexpr(level ==  Warn) {
      style = fmt::fg(fmt::terminal_color::yellow);
      prefix = "[W]";
    }
    if constexpr(level == Error) {
      style = fmt::fg(fmt::terminal_color::magenta);
      prefix = "[E]";
    }
    if constexpr(level == Fatal) {
      style = fmt::fg(fmt::terminal_color::red);
      prefix = "[F]";
    }

    const auto& style_ref = style;
    fmt::print(style_ref, "{} {}\n", prefix, fmt::format(format, std::forward<Args>(args)...));
  }
}

template<typename... Args>
inline void Assert(bool condition, Args... args) {
  if(!condition) {
    Log<Fatal>(std::forward<Args>(args)...);
    std::exit(-1);
  }
}

} // namespace nba

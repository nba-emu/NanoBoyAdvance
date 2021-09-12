/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>
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
inline void Log(std::string_view format, Args... args) {
  if constexpr((detail::kLogMask & level) != 0) {
    char const* prefix = "[?]";

    if constexpr(level == Trace) prefix = "\e[36m[T]";
    if constexpr(level == Debug) prefix = "\e[34m[D]";
    if constexpr(level ==  Info) prefix = "\e[37m[I]";
    if constexpr(level ==  Warn) prefix = "\e[33m[W]";
    if constexpr(level == Error) prefix = "\e[35m[E]";
    if constexpr(level == Fatal) prefix = "\e[31m[F]";

    fmt::print("{} {}\n", prefix, fmt::format(format, args...));
  }
}

template<typename... Args>
inline void Assert(bool condition, Args... args) {
  if (!condition) {
    Log<Fatal>(args...);
    std::exit(-1);
  }
}

} // namespace nba

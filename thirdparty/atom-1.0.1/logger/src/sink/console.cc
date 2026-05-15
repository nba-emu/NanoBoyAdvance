// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#include <atom/logger/sink/console.hh>
#include <fmt/color.h>

namespace atom {

  void LoggerConsoleSink::AppendImpl(Logger::Message const& message) {
    const auto [level, time, component, text] = message;

    fmt::text_style text_style{};

    switch (level) {
      case Info:  text_style = fmt::fg(fmt::color::cornflower_blue); break;
      case Warn:  text_style = fmt::fg(fmt::color::yellow) | fmt::emphasis::bold; break;
      case Error: text_style = fmt::fg(fmt::color::red) | fmt::emphasis::bold; break;
      case Fatal: text_style = fmt::fg(fmt::color::magenta) | fmt::emphasis::bold; break;
      default: break;
    }

    const char* level_str = "?";

    switch (level) {
      case Trace: level_str = "T"; break;
      case Debug: level_str = "D"; break;
      case Info:  level_str = "I"; break;
      case Warn:  level_str = "W"; break;
      case Error: level_str = "E"; break;
      case Fatal: level_str = "F"; break;
      default: break;
    }

    fmt::print(text_style, "[{}] [{:02}:{:02}:{:02}] ({})\t {}\n", level_str, time.hour, time.minute, time.second, component.value_or("Unknown"), text);
  }

} // namespace atom

// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#include <atom/logger/sink/file.hh>
#include <atom/panic.hh>

namespace atom {

  LoggerFileSink::LoggerFileSink(std::string const& path) {
    file = std::fopen(path.c_str(), "w");

    if (file == nullptr) {
      ATOM_PANIC("Could not open log file: {}", path);
    }
  }

  LoggerFileSink::~LoggerFileSink() {
    std::fclose(file);
  }

  void LoggerFileSink::AppendImpl(Logger::Message const& message) {
    const auto [level, time, component, text] = message;

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

    fmt::print(file, "[{}] [{:02}:{:02}:{:02}] ({})\t {}\n", level_str, time.hour, time.minute, time.second, component.value_or("Unknown"), text);
  }

} // namespace atom

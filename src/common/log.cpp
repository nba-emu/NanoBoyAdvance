/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "log.hpp"

#ifdef WIN32
#include <stdio.h>
#include <windows.h>
#endif

namespace common::logger {

auto trim_filepath(const char* file) -> std::string {
  auto tmp = std::string{file};
#ifdef WIN32
  auto pos = tmp.find("\\source\\");
#else
  auto pos = tmp.find("/source/");
#endif
  if (pos == std::string::npos) {
    return "???";
  }
  return tmp.substr(pos);
}

void init() {
#ifdef WIN32
  // We require ANSI escape sequences for colored output.
  // Starting with Windows 10 they're supported, but must be activated manually.
  auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (handle != INVALID_HANDLE_VALUE) {
    DWORD mode;
    if (GetConsoleMode(handle, &mode)) {
      mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(handle, mode);
    }
  }
  
  // Workaround for Mingw-w64 build with no command line window.
  // The application will crash otherwise when flushing to a non-existent stdout.
  // TODO: replace with something less hacky, or at least detect when it's actually necessary.
#ifdef NDEBUG
  freopen ("log.txt", "w", stdout);
#endif

#endif
}

void append(Level level,
            const char* file,
            const char* function,
            int line,
            std::string const& message) {
  std::string prefix;

  switch (level) {
    case Level::Trace:
      prefix = "\e[36m[T]";
      break;
    case Level::Debug:
      prefix = "\e[34m[D]";
      break;
    case Level::Info:
      prefix = "\e[37m[I]";
      break;
    case Level::Warn:
      prefix = "\e[33m[W]";
      break;
    case Level::Error:
      prefix = "\e[35m[E]";
      break;
    case Level::Fatal:
      prefix = "\e[31m[F]";
      break;
  }

  fmt::print("{0} {1}:{2} [{3}]: {4}\e[39m\n", prefix, trim_filepath(file), line, function, message);
}

} // namespace common::logger

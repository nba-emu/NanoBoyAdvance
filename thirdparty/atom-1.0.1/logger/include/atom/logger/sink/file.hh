// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <atom/logger/logger.hh>
#include <cstdio>
#include <string>

namespace atom {

  /// Logs messages into a file
  class LoggerFileSink final : public Logger::SinkBase {
    public:
      /**
       * Creates a new file sink. The current contents of the log file are discarded.
       * @param path the path to the log file
       */
      explicit LoggerFileSink(std::string const& path);

     ~LoggerFileSink() override;

    protected:
      void AppendImpl(Logger::Message const& message) override;

    private:
      std::FILE* file;
  };

} // namespace atom

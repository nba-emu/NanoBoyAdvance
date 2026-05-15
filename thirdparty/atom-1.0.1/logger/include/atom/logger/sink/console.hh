// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <atom/logger/logger.hh>

namespace atom {

  /// Logs colored messages to the process's standard output handle
  class LoggerConsoleSink final : public Logger::SinkBase {
    protected:
      void AppendImpl(Logger::Message const& message) override;
  };

} // namespace atom

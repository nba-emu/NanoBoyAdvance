// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <algorithm>
#include <ctime>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <fmt/format.h>
#include <vector>

namespace atom {

  /**
   * Enumeration of available logging levels.
   */
  enum Level {
    Trace =  1, ///< Not enabled in release builds
    Debug =  2, ///< Not enabled in release builds
    Info  =  4,
    Warn  =  8,
    Error = 16,
    Fatal = 32,

    All = Trace | Debug | Info | Warn | Error | Fatal ///< All logging levels enabled
  };

  /**
   * Maintains a list of enabled and disabled log {@link #Level}s.
   * Allows querying enabled log levels and enabling or disabling specific levels.
   */
  class RuntimeLogLevelList {
    public:
      /// @returns the bitset of of enabled log {@link #Level}s
      [[nodiscard]] int GetLogMask() const {
        return runtime_log_mask;
      }

      /// Sets the bitset of enabled log {@link #Level}s
      void SetLogMask(int mask) {
        runtime_log_mask = mask;
      }

      /// @returns whether the specified log {@link #Level} is enabled
      [[nodiscard]] bool GetLogLevelEnable(Level level) const {
        return runtime_log_mask & static_cast<int>(level);
      }

      /**
       * Enable or disable a log {@link #Level}
       * @param level  the log level
       * @param enable whether the log level will be enabled or disabled
       */
      void SetLogLevelEnable(Level level, bool enable) {
        if(enable) {
          runtime_log_mask |= static_cast<int>(level);
        } else {
          runtime_log_mask &= ~static_cast<int>(level);
        }
      }

    private:
      int runtime_log_mask = All; ///< bitset of currently enabled log levels
  };

  /**
   * A named or unnamed logger.
   * Allows logging formatted messages to one or multiple logging sinks (@see #SinkBase) (e.g. console or file sink).
   * Log {@link #Level}s may be enabled or disabled during runtime, however the Trace and Debug log levels are
   * always disabled in release builds.
   */
  class Logger : public RuntimeLogLevelList {
    public:
      /// A structured log message containing log level, time, source component and message text.
      struct Message {
        Level level;

        struct Time {
          int hour;
          int minute;
          int second;
        } time;

        std::optional<std::string_view> component;

        std::string_view text;
      };

      /// Abstract base class for all logger sinks.
      class SinkBase : public RuntimeLogLevelList {
        public:
          virtual ~SinkBase() = default;

          /**
           * Send a structured log message to this sink.
           * The message is filtered according to its log level and passed to {@link #AppendImpl}
           * only if the log level is enabled for this sink.
           * @param message the structured log message
           */
          void Append(Message const& message) {
            if(GetLogLevelEnable(message.level)) {
              AppendImpl(message);
            }
          }

        protected:
          /**
           * Detail for sending a structured log message to this sink.
           * Override this method to receive logger messages.
           * @param message the structured log message
           */
          virtual void AppendImpl(Message const& message) = 0;
      };

      /// A collection of logger sinks
      class SinkCollection : public SinkBase {
        public:
          /// Add a sink to the collection
          void Install(std::shared_ptr<SinkBase> const& sink) {
            sinks.push_back(sink);
          }

          /// Remove a sink from the collection
          void Remove(std::shared_ptr<SinkBase> const& sink) {
            sinks.erase(std::remove(sinks.begin(), sinks.end(), sink), sinks.end());
          }

          /// @returns a span referring to all registered sinks
          [[nodiscard]] std::span<const std::shared_ptr<SinkBase>> AsSpan() {
            return sinks;
          }

        protected:
          void AppendImpl(Message const& message) override {
            for(auto& sink : sinks) {
              sink->Append(message);
            }
          }

        private:
          std::vector<std::shared_ptr<SinkBase>> sinks; ///< list of all registered sinks
      };

      /// Create a nameless logger with a new sink collection
      Logger() : sink_collection{std::make_shared<SinkCollection>()} {}

      /// Create a names logger with a new sink collection
      explicit Logger(std::string_view name)
        : name{name}, sink_collection{std::make_shared<SinkCollection>()} {}

      /// Create a logger with a preexisting sink collection and an optional name
      explicit Logger(
        std::shared_ptr<SinkCollection> sink_collection,
        std::optional<std::string_view> name = std::nullopt
      )   : name{name}, sink_collection{std::move(sink_collection)} {}

      /// @returns the sink collection used by this logger
      [[nodiscard]] std::shared_ptr<SinkCollection> GetSinkCollection() const {
        return sink_collection;
      }

      /// Replace the currently used sink collection with a different one
      void SetSinkCollection(std::shared_ptr<SinkCollection> new_sink_collection) {
        this->sink_collection = std::move(new_sink_collection);
      }

      /**
       * Log a formatted message to this logger
       * @tparam level the log level
       * @tparam Args
       * @param format the message format
       * @param args the variable arguments for formatting the message
       */
      template<Level level, typename... Args>
      void Log(std::string_view format, Args&&... args) const {
        if constexpr(k_build_log_mask & level) {
          if(GetLogLevelEnable(level)) {
            std::string text = fmt::format(fmt::runtime(format), std::forward<Args>(args)...);

            SendMessage({level, GetCurrentTime(), name, text});
          }
        }
      }

      /// Add a sink to the currently used sink collection
      void InstallSink(std::shared_ptr<SinkBase> const& sink) {
        sink_collection->Install(sink);
      }

      /// Remove a sink from the currently used sink collection
      void RemoveSink(std::shared_ptr<SinkBase> const& sink) {
        sink_collection->Remove(sink);
      }

    private:
#if defined(NDEBUG)
      static constexpr int k_build_log_mask = Info | Warn | Error | Fatal;
#else
      static constexpr int k_build_log_mask = All;
#endif

      void SendMessage(Message const& message) const {
        sink_collection->Append(message);
      }

      static Message::Time GetCurrentTime() {
        // @todo std::localtime() is not thread-safe.
        std::time_t time = std::time(nullptr);
        std::tm* local_time = std::localtime(&time);
        int hour = local_time->tm_hour;
        int minute = local_time->tm_min;
        int second = local_time->tm_sec;

        return {hour, minute, second};
      }

      std::optional<std::string> name;
      std::shared_ptr<SinkCollection> sink_collection;
  };

  /// @returns the default logger (as used by ATOM_INFO, ATOM_WARN macros etc.)
  Logger& get_logger();

  /// @returns a named logger. A logger is created if the name is not known yet.
  Logger& get_named_logger(std::string const& name);

} // namespace atom

#define ATOM_LOG(level, format, ...) atom::get_logger().Log<level>(format, ## __VA_ARGS__);

#define ATOM_TRACE(format, ...) ATOM_LOG(atom::Trace, format, ## __VA_ARGS__)
#define ATOM_DEBUG(format, ...) ATOM_LOG(atom::Debug, format, ## __VA_ARGS__)
#define ATOM_INFO(format, ...)  ATOM_LOG(atom::Info,  format, ## __VA_ARGS__)
#define ATOM_WARN(format, ...)  ATOM_LOG(atom::Warn,  format, ## __VA_ARGS__)
#define ATOM_ERROR(format, ...) ATOM_LOG(atom::Error, format, ## __VA_ARGS__)
#define ATOM_FATAL(format, ...) ATOM_LOG(atom::Fatal, format, ## __VA_ARGS__)

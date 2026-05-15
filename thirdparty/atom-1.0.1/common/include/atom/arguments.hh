// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <algorithm>
#include <atom/detail/parse_utils.hh>
#include <atom/panic.hh>
#include <filesystem>
#include <fmt/format.h>
#include <optional>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace atom {

  /**
   * Command line arguments parser
   */
  class Arguments {
    public:
      /// Describes the version of an application in the semantic versioning (SemVer) format.
      struct Version {
        int major;
        int minor;
        int patch;
      };

      /**
       * Create a command line argument parser for an application
       * @param app_name The name of the application
       * @param app_description A short description of the application
       * @param app_version The version of the application according to semantic versioning (SemVer).
       */
      Arguments(std::string_view app_name, std::string_view app_description, Version app_version)
          : m_app_name{app_name}
          , m_app_description{app_description}
          , m_app_version{app_version} {
      }

      template<typename T>
      void RegisterArgument(
        T& value,
        bool optional,
        std::string_view long_name,
        std::optional<std::string_view> description = std::nullopt,
        std::optional<std::string_view> placeholder = std::nullopt
      ) {
        m_longest_argument_length = std::max(m_longest_argument_length, long_name.size());

        m_argument_list.emplace_back(
          GetArgumentTypeFromCppType<T>(),
          optional,
          std::string{long_name},
          description.has_value() ? std::optional{std::string{description.value()}} : std::nullopt,
          placeholder.has_value() ? std::optional{std::string{placeholder.value()}} : std::nullopt,
          &value
        );
      }

      void RegisterFile(std::string_view name, bool optional) {
        if(!optional) {
          if(!m_file_list.empty() && m_file_list.back().optional) {
            ATOM_PANIC("optional file argument must not be followed by a required file argument");
          }
          m_required_file_count++;
        }

        m_file_list.emplace_back(std::string{name}, optional);
      }

      void AllowAdditionalFiles(std::string_view name) {
        m_allow_additional_files = true;
        m_additional_files_name = name;
      }

      bool Parse(int argc, char** argv, std::vector<const char*>* files = nullptr) {
        std::unordered_set<std::string> arguments_seen;
        bool help_requested = false;
        size_t file_count = 0;

        for(int i = 1; i < argc; i++) {
          std::string arg = argv[i];

          if(!arg.starts_with("--")) {
            if(files) {
              files->push_back(argv[i]);
            }
            file_count++;
            continue;
          }

          if(arg == "--help") {
            help_requested = true;
            continue;
          }

          const size_t equals_sign_pos = arg.find('=');

          std::string arg_name;
          std::optional<std::string> arg_value;

          if(equals_sign_pos != std::string::npos) {
            arg_name  = arg.substr(2, equals_sign_pos - 2);
            arg_value = arg.substr(equals_sign_pos + 1);
          } else {
            arg_name  = arg.substr(2);
          }

          const auto arg_data = std::find_if(
            m_argument_list.begin(), m_argument_list.end(), [&](const Argument& arg) {return arg.long_name == arg_name;});

          if(arg_data == m_argument_list.end()) {
            Usage(argc, argv);
            return false;
          }

          const auto GetArgValue = [&]() -> std::optional<std::string> {
            if(arg_value.has_value()) return arg_value;
            if(i + 1 < argc)          return std::string{argv[++i]};
            return std::nullopt;
          };

          void* data = arg_data->data;

          switch(arg_data->type) {
            case Argument::Type::String: {
              std::optional<std::string> value = GetArgValue();
              if(!value.has_value()) {
                Usage(argc, argv);
                return false;
              }
              *(std::string*)data = value.value();
              break;
            }
            case Argument::Type::Boolean: {
              // TODO(fleroviux): support `--arg <value>` syntax
              if(arg_value.has_value()) {
                std::string v = arg_value.value();
                     if(v == "y" || v == "1" || v == "true" ) *(bool*)data = true;
                else if(v == "n" || v == "0" || v == "false") *(bool*)data = false;
                else { Usage(argc, argv); return false; }
              } else {
                *(bool*)data = true;
              }
              break;
            }
            case Argument::Type::Integer: {
              std::optional<std::string> maybe_value = GetArgValue();
              if(!maybe_value.has_value()) {
                Usage(argc, argv);
                return false;
              }

              std::optional<int> maybe_number = detail::parse_numeric_string<int>(maybe_value.value());
              if(!maybe_number.has_value()) {
                Usage(argc, argv);
                return false;
              }
              *(int*)data = maybe_number.value();
              break;
            }
            default: ATOM_PANIC("unhandled argument type");
          }

          arguments_seen.insert(arg_name);
        }

        for(const auto& argument : m_argument_list) {
          if(!argument.optional && !arguments_seen.contains(argument.long_name)) {
            Usage(argc, argv);
            return false;
          }
        }

        if(file_count < m_required_file_count || (!m_allow_additional_files && file_count > m_file_list.size())) {
          Usage(argc, argv);
          return false;
        }

        if(help_requested) {
          Usage(argc, argv);
        }

        return true;
      }

      void Usage(int argc, char** argv) {
        const Version& version = m_app_version;
        fmt::print("{} {}.{}.{}\n\n{}\n\n", m_app_name, version.major, version.minor, version.patch, m_app_description);

        {
          std::optional<std::string> executable_name;
          if(argc > 0) {
            executable_name = std::filesystem::path{argv[0]}.filename().string();
          }
          fmt::print("Usage: {}", executable_name.value_or("(null)"));

          for(const auto& argument : m_argument_list) {
            fmt::print(" ");

            if(argument.optional) fmt::print("[");

            fmt::print("--{}", argument.long_name);
            if(argument.type != Argument::Type::Boolean) {
              fmt::print("=<{}>", argument.placeholder.value_or("value"));
            }

            if(argument.optional) fmt::print("]");
          }

          for(const auto& file : m_file_list) {
            fmt::print(" ");

            if(file.optional) fmt::print("[");

            fmt::print("{}", file.name);

            if(file.optional) fmt::print("]");
          }

          if(m_allow_additional_files) {
            fmt::print(" [{}...]", m_additional_files_name);
          }

          fmt::print("\n\n");
        }

        if(!m_argument_list.empty()){
          const size_t padding = m_longest_argument_length + 2;
          const std::string format_head = fmt::format(" {{}}:    {{:>{}}}:\n\n", padding);
          const std::string format_row  = fmt::format(" --{{:{}}}  {{}}\n", padding);

          fmt::print(fmt::runtime(format_head), "Option", "Meaning");
          fmt::print(fmt::runtime(format_row), "help", "Displays this help text");

          for(const auto& argument : m_argument_list) {
            fmt::print(fmt::runtime(format_row), argument.long_name, argument.description.value_or(""));
          }
        }
      }

    private:
      struct Argument {
        enum class Type {
          String,
          Boolean,
          Integer
        } type;

        bool optional;
        std::string long_name;
        std::optional<std::string> description;
        std::optional<std::string> placeholder;
        void* data;

        Argument(
          Type type,
          bool optional,
          std::string long_name,
          std::optional<std::string> description,
          std::optional<std::string> placeholder,
          void* data
        )   : type{type}
            , optional{optional}
            , long_name{std::move(long_name)}
            , description{std::move(description)}
            , placeholder{std::move(placeholder)}
            , data{data} {
        }
      };

      struct File {
        std::string name;
        bool optional;

        File(std::string name, bool optional) : name{std::move(name)}, optional{optional} {}
      };

      template<typename T>
      [[nodiscard]] static constexpr Argument::Type GetArgumentTypeFromCppType() {
        if constexpr(std::is_same_v<T, std::string>) return Argument::Type::String;
        if constexpr(std::is_same_v<T, bool>)        return Argument::Type::Boolean;
        if constexpr(std::is_same_v<T, int>)         return Argument::Type::Integer;
      }

      std::string m_app_name;
      std::string m_app_description;
      Version m_app_version;

      std::vector<Argument> m_argument_list{};
      std::vector<File> m_file_list{};
      bool m_allow_additional_files{};
      std::string m_additional_files_name{};
      size_t m_longest_argument_length{0};
      size_t m_required_file_count{0};
  };

} // namespace atom

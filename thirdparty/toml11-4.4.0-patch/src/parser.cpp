#include <toml11/parser.hpp>
#include <toml11/types.hpp>

#if ! defined(TOML11_COMPILE_SOURCES)
#error "Define `TOML11_COMPILE_SOURCES` before compiling source code!"
#endif

namespace toml
{

template result<basic_value<type_config>, std::vector<error_info>> try_parse<type_config>(std::vector<unsigned char>, std::string, spec);
template result<basic_value<type_config>, std::vector<error_info>> try_parse<type_config>(std::istream&, std::string, spec);
template result<basic_value<type_config>, std::vector<error_info>> try_parse<type_config>(std::string, spec);
template result<basic_value<type_config>, std::vector<error_info>> try_parse<type_config>(FILE*, std::string, spec);
template result<basic_value<type_config>, std::vector<error_info>> try_parse_str<type_config>(std::string, spec, cxx::source_location);

template basic_value<type_config> parse<type_config>(std::vector<unsigned char>, std::string, spec);
template basic_value<type_config> parse<type_config>(std::istream&, std::string, spec);
template basic_value<type_config> parse<type_config>(std::string, spec);
template basic_value<type_config> parse<type_config>(FILE*, std::string, spec);
template basic_value<type_config> parse_str<type_config>(std::string, spec, cxx::source_location);

template result<basic_value<ordered_type_config>, std::vector<error_info>> try_parse<ordered_type_config>(std::vector<unsigned char>, std::string, spec);
template result<basic_value<ordered_type_config>, std::vector<error_info>> try_parse<ordered_type_config>(std::istream&, std::string, spec);
template result<basic_value<ordered_type_config>, std::vector<error_info>> try_parse<ordered_type_config>(std::string, spec);
template result<basic_value<ordered_type_config>, std::vector<error_info>> try_parse<ordered_type_config>(FILE*, std::string, spec);
template result<basic_value<ordered_type_config>, std::vector<error_info>> try_parse_str<ordered_type_config>(std::string, spec, cxx::source_location);

template basic_value<ordered_type_config> parse<ordered_type_config>(std::vector<unsigned char>, std::string, spec);
template basic_value<ordered_type_config> parse<ordered_type_config>(std::istream&, std::string, spec);
template basic_value<ordered_type_config> parse<ordered_type_config>(std::string, spec);
template basic_value<ordered_type_config> parse<ordered_type_config>(FILE*, std::string, spec);
template basic_value<ordered_type_config> parse_str<ordered_type_config>(std::string, spec, cxx::source_location);

#if defined(TOML11_HAS_FILESYSTEM)
template cxx::enable_if_t<std::is_same<std::filesystem::path, std::filesystem::path>::value, result<basic_value<type_config>,         std::vector<error_info>>> try_parse<type_config,         std::filesystem::path>(const std::filesystem::path&, spec);
template cxx::enable_if_t<std::is_same<std::filesystem::path, std::filesystem::path>::value, result<basic_value<ordered_type_config>, std::vector<error_info>>> try_parse<ordered_type_config, std::filesystem::path>(const std::filesystem::path&, spec);
template cxx::enable_if_t<std::is_same<std::filesystem::path, std::filesystem::path>::value, basic_value<type_config>                                         > parse    <type_config,         std::filesystem::path>(const std::filesystem::path&, spec);
template cxx::enable_if_t<std::is_same<std::filesystem::path, std::filesystem::path>::value, basic_value<ordered_type_config>                                 > parse    <ordered_type_config, std::filesystem::path>(const std::filesystem::path&, spec);
#endif // filesystem

} // toml

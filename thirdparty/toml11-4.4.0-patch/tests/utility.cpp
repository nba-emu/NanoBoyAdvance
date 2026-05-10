#include "doctest.h"

#include "utility.hpp"

#include <ostream>
#include <string>

#include <iostream>

void test_scan_success(const toml::detail::scanner_base& s,
        const std::string& in, const std::string& out)
{
    auto loc = toml::detail::make_temporary_location(in);
    const auto reg = s.scan(loc);
    CHECK_UNARY(reg.is_ok());
    CHECK_EQ(reg.as_string(), out);
}
void test_scan_failure(const toml::detail::scanner_base& s,
        const std::string& in)
{
    auto loc = toml::detail::make_temporary_location(in);
    const auto reg = s.scan(loc);
    CHECK_UNARY_FALSE(reg.is_ok());
}

namespace toml
{

std::ostream& operator<<(std::ostream& os, const integer_format_info& fmt)
{
    os << "integer_format_info{";
    os << "fmt = "       << fmt.fmt       << ", ";
    os << "uppercase = " << fmt.uppercase << ", ";
    os << "width = "     << fmt.width     << ", ";
    os << "spacer = "    << fmt.spacer    << ", ";
    os << "suffix = \""  << fmt.suffix    << "\"}";
    return os;
}
std::ostream& operator<<(std::ostream& os, const floating_format_info& fmt)
{
    os << "floating_format_info{";
    os << "fmt = "      << fmt.fmt    << ", ";
    os << "prec = "     << fmt.prec   << ", ";
    os << "suffix = \"" << fmt.suffix << "\"}";
    return os;
}
std::ostream& operator<<(std::ostream& os, const string_format_info& fmt)
{
    os << "string_format_info{";
    os << "fmt = "                << fmt.fmt                << ", ";
    os << "start_with_newline = " << fmt.start_with_newline << "}";
    return os;
}
std::ostream& operator<<(std::ostream& os, const offset_datetime_format_info& fmt)
{
    os << "offset_datetime_format_info{";
    os << std::boolalpha;
    os << "delimiter = "           << fmt.delimiter << ", ";
    os << "has_seconds = "         << fmt.has_seconds << ", ";
    os << "subsecond_precision = " << fmt.subsecond_precision << "}";
    return os;
}
std::ostream& operator<<(std::ostream& os, const local_datetime_format_info& fmt)
{
    os << "local_datetime_format_info{";
    os << std::boolalpha;
    os << "delimiter = "           << fmt.delimiter << ", ";
    os << "has_seconds = "         << fmt.has_seconds << ", ";
    os << "subsecond_precision = " << fmt.subsecond_precision << "}";
    return os;
}
std::ostream& operator<<(std::ostream& os, const local_date_format_info&)
{
    os << "local_date_format_info{}";
    return os;
}
std::ostream& operator<<(std::ostream& os, const local_time_format_info& fmt)
{
    os << "local_time_format_info{";
    os << std::boolalpha;
    os << "has_seconds = "         << fmt.has_seconds << ", ";
    os << "subsecond_precision = " << fmt.subsecond_precision << "}";
    return os;
}
std::ostream& operator<<(std::ostream& os, const array_format_info& fmt)
{
    os << "array_format_info{";
    os << "fmt = "            << fmt.fmt            << ", ";
    os << "indent_type = "    << fmt.indent_type    << ", ";
    os << "body_indent = "    << fmt.body_indent    << ", ";
    os << "closing_indent = " << fmt.closing_indent << "}";
    return os;
}
std::ostream& operator<<(std::ostream& os, const table_format_info& fmt)
{
    os << "table_format_info{";
    os << "fmt = "            << fmt.fmt << ", ";
    os << "indent_type = "    << fmt.indent_type    << ", ";
    os << "name_indent = "    << fmt.name_indent    << ", ";
    os << "body_indent = "    << fmt.body_indent    << ", ";
    os << "closing_indent = " << fmt.closing_indent << "}";
    return os;
}

} // toml

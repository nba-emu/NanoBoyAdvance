#ifndef TOML11_TEST_UTILITY_HPP
#define TOML11_TEST_UTILITY_HPP

#include <toml11/context.hpp>
#include <toml11/format.hpp>
#include <toml11/parser.hpp>
#include <toml11/region.hpp>
#include <toml11/scanner.hpp>
#include <toml11/serializer.hpp>
#include <toml11/value.hpp>

#include "doctest.h"

#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include <iostream>

template<typename TC>
toml::basic_value<TC> const& as_const(toml::basic_value<TC>& v) noexcept
{
    return v;
}

template<typename ... Ts>
std::vector<std::string> comments(Ts ... args)
{
    return std::vector<std::string>{std::move(args)...};
}

void test_scan_success(const toml::detail::scanner_base& s,
        const std::string& in, const std::string& out);
void test_scan_failure(const toml::detail::scanner_base& s,
        const std::string& in);

template<toml::value_t VT, typename T, typename Format, typename TC>
void toml11_test_parse_success(
        std::string in, const T& out,
        const std::vector<std::string>& com,
        const Format& fmt,
        toml::detail::context<TC> ctx)
{
    auto loc = toml::detail::make_temporary_location(in);

    const auto res = toml::detail::parse_value(loc, ctx);
    if(res.is_err())
    {
        std::cerr << toml::format_error(res.unwrap_err()) << std::endl;
    }
    REQUIRE_UNARY(res.is_ok());

    const auto val = res.unwrap();

    REQUIRE(val.is(VT));
    REQUIRE_EQ(val.type(), VT);

    CHECK_EQ(val.template as<VT>(), out);

    const std::vector<std::string> com_actual(
            val.comments().begin(), val.comments().end());
    CHECK_EQ(com_actual, com);

    const auto fmt_actual = val.template as_fmt<VT>();
    CHECK_EQ(fmt_actual, fmt);
}

template<typename F, typename TC>
void toml11_test_parse_failure(F fn, std::string in, toml::detail::context<TC> ctx)
{
    using namespace toml::detail;
    auto loc = toml::detail::make_temporary_location(in);
    const auto res = fn(loc, ctx);
    REQUIRE_UNARY(res.is_err());

    ctx.report_error(res.unwrap_err());

    std::cerr << "-------- error messages about: `" << in.substr(0, in.find('\n')) << "` --------" << std::endl;
    for(const auto& e : ctx.errors())
    {
        std::cerr << toml::format_error(e) << std::endl;
    }
    std::cerr << "-------- end --------" << std::endl;
}

namespace toml
{
std::ostream& operator<<(std::ostream& os, const integer_format_info&);
std::ostream& operator<<(std::ostream& os, const floating_format_info&);
std::ostream& operator<<(std::ostream& os, const string_format_info&);
std::ostream& operator<<(std::ostream& os, const offset_datetime_format_info&);
std::ostream& operator<<(std::ostream& os, const local_datetime_format_info&);
std::ostream& operator<<(std::ostream& os, const local_date_format_info&);
std::ostream& operator<<(std::ostream& os, const local_time_format_info&);
std::ostream& operator<<(std::ostream& os, const array_format_info&);
std::ostream& operator<<(std::ostream& os, const table_format_info&);

} // toml
#endif// TOML11_TEST_UTILITY_HPP

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/location.hpp>

TEST_CASE("testing location")
{
    const auto first = toml::detail::make_temporary_location("hogefuga");

    auto loc = first;
    loc.advance(4);

    CHECK_UNARY(loc.is_ok());
    CHECK_UNARY( ! loc.eof());
    CHECK_EQ(loc.current(), 'f');
    CHECK_EQ(loc.line_number(), 1);
    CHECK_EQ(loc.column_number(), 5);

    const auto loc2 = loc;
    CHECK_EQ(loc, loc2);
    CHECK_UNARY(loc == loc2);
    CHECK_UNARY(!(loc != loc2));

    loc.advance(4);
    CHECK_UNARY(loc.is_ok());
    CHECK_UNARY(loc.eof());
    CHECK_EQ(loc.current(), '\0');
    CHECK_EQ(loc.line_number(), 1);
    CHECK_EQ(loc.column_number(), 9);

    CHECK_NE(loc, loc2);
    CHECK_UNARY(!(loc == loc2));
    CHECK_UNARY(loc != loc2);

    CHECK_EQ(toml::detail::count(first, loc, 'e'), 1);
    CHECK_EQ(toml::detail::count(first, loc, 'g'), 2);
}

TEST_CASE("testing multiline location")
{
    const auto first = toml::detail::make_temporary_location("hoge\nfuga");
    auto loc = first;
    loc.advance(5);

    CHECK_UNARY(loc.is_ok());
    CHECK_UNARY( ! loc.eof());
    CHECK_EQ(loc.current(), 'f');
    CHECK_EQ(loc.line_number(), 2);
    CHECK_EQ(loc.column_number(), 1);

    const auto newline_r = toml::detail::rfind(first, loc, '\n');
    CHECK_UNARY(newline_r.is_ok());

    const auto newline = newline_r.as_ok();
    CHECK_EQ(newline.current(), '\n');

    loc.advance(4);
    CHECK_UNARY(loc.is_ok());
    CHECK_UNARY(loc.eof());
    CHECK_EQ(loc.current(), '\0');
    CHECK_EQ(loc.line_number(), 2);
    CHECK_EQ(loc.column_number(), 5);
}

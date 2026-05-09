#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/parser.hpp>
#include <toml11/types.hpp>

TEST_CASE("testing fractional float")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));

    auto fmt = [](std::size_t prec) {
        toml::floating_format_info f;
        f.fmt = toml::floating_format::fixed;
        f.prec = prec;
        return f;
    };

    toml11_test_parse_success<toml::value_t::floating>("1.0",                1.0           , comments(), fmt( 1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("0.1",                0.1           , comments(), fmt( 1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("0.001",              0.001         , comments(), fmt( 3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("0.100",              0.1           , comments(), fmt( 3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("+3.14",              3.14          , comments(), fmt( 2), ctx);
    toml11_test_parse_success<toml::value_t::floating>("-3.14",             -3.14          , comments(), fmt( 2), ctx);
    toml11_test_parse_success<toml::value_t::floating>("3.1415_9265_3589",   3.141592653589, comments(), fmt(12), ctx);
    toml11_test_parse_success<toml::value_t::floating>("+3.1415_9265_3589",  3.141592653589, comments(), fmt(12), ctx);
    toml11_test_parse_success<toml::value_t::floating>("-3.1415_9265_3589", -3.141592653589, comments(), fmt(12), ctx);
    toml11_test_parse_success<toml::value_t::floating>("123_456.789",        123456.789    , comments(), fmt( 3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("+123_456.789",       123456.789    , comments(), fmt( 3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("-123_456.789",      -123456.789    , comments(), fmt( 3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("+0.0",               0.0           , comments(), fmt( 1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("-0.0",              -0.0           , comments(), fmt( 1), ctx);
}

TEST_CASE("testing exponents")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));

    auto fmt = [](std::size_t prec) {
        toml::floating_format_info f;
        f.fmt = toml::floating_format::scientific;
        f.prec = prec;
        return f;
    };

    toml11_test_parse_success<toml::value_t::floating>("1e10",       1e10   , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("1e+10",      1e10   , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("1e-10",      1e-10  , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("+1e10",      1e10   , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("+1e+10",     1e10   , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("+1e-10",     1e-10  , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("-1e10",      -1e10  , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("-1e+10",     -1e10  , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("-1e-10",     -1e-10 , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("123e-10",    123e-10, comments(), fmt(3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("1E10",       1e10   , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("1E+10",      1e10   , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("1E-10",      1e-10  , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("123E-10",    123e-10, comments(), fmt(3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("1_2_3E-10",  123e-10, comments(), fmt(3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("1_2_3E-1_0", 123e-10, comments(), fmt(3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("+0e0",        0.0   , comments(), fmt(1), ctx);
    toml11_test_parse_success<toml::value_t::floating>("-0e0",       -0.0   , comments(), fmt(1), ctx);
}

TEST_CASE("testing fraction + exponents")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));

    auto fmt = [](std::size_t prec) {
        toml::floating_format_info f;
        f.fmt = toml::floating_format::scientific;
        f.prec = prec;
        return f;
    };

    toml11_test_parse_success<toml::value_t::floating>("6.02e23",          6.02e23,        comments(), fmt(3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("6.02e+23",         6.02e23,        comments(), fmt(3), ctx);
    toml11_test_parse_success<toml::value_t::floating>("1.112_650_06e-17", 1.11265006e-17, comments(), fmt(9), ctx);
}

TEST_CASE("testing +/-inf")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));

    {
        auto loc = toml::detail::make_temporary_location("inf");
        const auto res = toml::detail::parse_floating(loc, ctx);
        REQUIRE_UNARY(res.is_ok());
        const auto val = res.unwrap();
        REQUIRE_UNARY(val.is_floating());

        CHECK_UNARY(std::isinf(val.as_floating()));
        CHECK_UNARY(val.as_floating() > 0); // +
    }
    {
        auto loc = toml::detail::make_temporary_location("+inf");
        const auto res = toml::detail::parse_floating(loc, ctx);
        REQUIRE_UNARY(res.is_ok());
        const auto val = res.unwrap();
        REQUIRE_UNARY(val.is_floating());

        CHECK_UNARY(std::isinf(val.as_floating()));
        CHECK_UNARY(val.as_floating() > 0); // +
    }
    {
        auto loc = toml::detail::make_temporary_location("-inf");
        const auto res = toml::detail::parse_floating(loc, ctx);
        REQUIRE_UNARY(res.is_ok());
        const auto val = res.unwrap();
        REQUIRE_UNARY(val.is_floating());

        CHECK_UNARY(std::isinf(val.as_floating()));
        CHECK_UNARY(val.as_floating() < 0); // -
    }
}

TEST_CASE("testing +/-nan")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));

    {
        auto loc = toml::detail::make_temporary_location("nan");
        const auto res = toml::detail::parse_floating(loc, ctx);
        REQUIRE_UNARY(res.is_ok());
        const auto val = res.unwrap();
        REQUIRE_UNARY(val.is_floating());

        CHECK_UNARY(std::isnan(val.as_floating()));
    }
    {
        auto loc = toml::detail::make_temporary_location("+nan");
        const auto res = toml::detail::parse_floating(loc, ctx);
        REQUIRE_UNARY(res.is_ok());
        const auto val = res.unwrap();
        REQUIRE_UNARY(val.is_floating());

        CHECK_UNARY(std::isnan(val.as_floating()));
    }
    {
        auto loc = toml::detail::make_temporary_location("-nan");
        const auto res = toml::detail::parse_floating(loc, ctx);
        REQUIRE_UNARY(res.is_ok());
        const auto val = res.unwrap();
        REQUIRE_UNARY(val.is_floating());

        CHECK_UNARY(std::isnan(val.as_floating()));
    }
}

TEST_CASE("testing hexfloat")
{
    toml::spec s = toml::spec::v(1,0,0);
    s.ext_hex_float = true;
    toml::detail::context<toml::type_config> ctx(s);

    {
        auto loc = toml::detail::make_temporary_location("0xABCp-3");
        const auto res = toml::detail::parse_floating(loc, ctx);
        REQUIRE_UNARY(res.is_ok());
        const auto val = res.unwrap();
        REQUIRE_UNARY(val.is_floating());

        CHECK_EQ(val.as_floating(), 343.5);
    }
}

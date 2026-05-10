#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/parser.hpp>
#include <toml11/types.hpp>

TEST_CASE("testing null value extension")
{
    toml::spec spec = toml::spec::v(1,0,0);
    spec.ext_null_value = true;
    toml::detail::context<toml::type_config> ctx(spec);

    auto loc = toml::detail::make_temporary_location("null");

    const auto res = toml::detail::parse_value<toml::type_config>(loc, ctx);
    if(res.is_err())
    {
        std::cerr << format_error(res.unwrap_err()) << std::endl;
    }
    REQUIRE_UNARY(res.is_ok());
    const auto val = res.unwrap();

    REQUIRE_UNARY(val.is_empty());
}

TEST_CASE("testing null value extension OFF")
{
    toml::spec spec = toml::spec::v(1,0,0);
    toml::detail::context<toml::type_config> ctx(spec);

    auto loc = toml::detail::make_temporary_location("null");

    const auto res = toml::detail::parse_value<toml::type_config>(loc, ctx);
    CHECK_UNARY(res.is_err());
}

TEST_CASE("testing null value extension, w/ comments")
{
    toml::spec spec = toml::spec::v(1,0,0);
    spec.ext_null_value = true;
    toml::detail::context<toml::type_config> ctx(spec);

    auto loc = toml::detail::make_temporary_location(R"(a = null # comment)");

    toml::value table{toml::table()};
    const auto res = toml::detail::parse_table(loc, ctx, table);
    if(res.is_err())
    {
        std::cerr << format_error(res.unwrap_err()) << std::endl;
    }
    REQUIRE_UNARY(res.is_ok());

    CHECK_UNARY(table.contains("a"));
    const auto null = table.at("a");

    CHECK_UNARY(null.is_empty());
    CHECK_EQ(null.comments().size(), 1);
    CHECK_EQ(null.comments().at(0), "# comment");
}

TEST_CASE("testing null value extension, in an array")
{
    toml::spec spec = toml::spec::v(1,0,0);
    spec.ext_null_value = true;
    toml::detail::context<toml::type_config> ctx(spec);

    auto loc = toml::detail::make_temporary_location(R"(a = [1, null, 3, 4, 5])");

    toml::value table{toml::table()};
    const auto res = toml::detail::parse_table(loc, ctx, table);
    if(res.is_err())
    {
        std::cerr << format_error(res.unwrap_err()) << std::endl;
    }
    REQUIRE_UNARY(res.is_ok());

    CHECK_UNARY(table.contains("a"));
    const auto a = table.at("a");

    CHECK_UNARY(a.is_array());
    CHECK_EQ(a.as_array().size(), 5);

    CHECK_UNARY(a.as_array().at(0).is_integer());
    CHECK_UNARY(a.as_array().at(1).is_empty());
    CHECK_UNARY(a.as_array().at(2).is_integer());
    CHECK_UNARY(a.as_array().at(3).is_integer());
    CHECK_UNARY(a.as_array().at(4).is_integer());
}

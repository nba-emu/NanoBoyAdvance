#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/parser.hpp>
#include <toml11/types.hpp>

TEST_CASE("testing valid boolean")
{
    const toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    {
        auto loc = toml::detail::make_temporary_location("true");
        const auto res = toml::detail::parse_boolean(loc, ctx);
        REQUIRE_UNARY(res.is_ok());

        const auto val = res.unwrap();
        REQUIRE_UNARY(val.is_boolean());

        CHECK_EQ(val.as_boolean(), true);
    }
    {
        auto loc = toml::detail::make_temporary_location("false");
        const auto res = toml::detail::parse_boolean(loc, ctx);
        REQUIRE_UNARY(res.is_ok());

        const auto val = res.unwrap();
        REQUIRE_UNARY(val.is_boolean());

        CHECK_EQ(val.as_boolean(), false);
    }
}

TEST_CASE("testing invalid boolean")
{
    using namespace toml::detail;

    const context<toml::type_config> ctx(toml::spec::v(1,0,0));

    toml11_test_parse_failure(parse_boolean<toml::type_config>, "True",  ctx);
    toml11_test_parse_failure(parse_boolean<toml::type_config>, "TRUE",  ctx);
    toml11_test_parse_failure(parse_boolean<toml::type_config>, "False", ctx);
    toml11_test_parse_failure(parse_boolean<toml::type_config>, "FALSE", ctx);
}

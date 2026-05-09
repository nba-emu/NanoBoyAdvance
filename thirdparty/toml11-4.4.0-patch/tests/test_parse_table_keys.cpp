#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <iostream>
#include "utility.hpp"
#include <toml11/parser.hpp>
#include <toml11/types.hpp>

#include "doctest.h"

TEST_CASE("testing table keys")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    {
        auto loc = toml::detail::make_temporary_location("[one-key]");
        const auto res = toml::detail::parse_table_key<toml::type_config>(loc, ctx);
        REQUIRE_UNARY(res.is_ok());

        const auto val = std::get<0>(res.unwrap());
        REQUIRE_UNARY(val.size() == 1);
        REQUIRE_UNARY(val.at(0)  == "one-key");
    }

    {
        auto loc = toml::detail::make_temporary_location("[many.keys]");
        const auto res = toml::detail::parse_table_key<toml::type_config>(loc, ctx);
        REQUIRE_UNARY(res.is_ok());

        const auto val = std::get<0>(res.unwrap());
        REQUIRE_UNARY(val.size() == 2);
        REQUIRE_UNARY(val.at(0)  == "many");
        REQUIRE_UNARY(val.at(1)  == "keys");
    }

    {
        auto loc = toml::detail::make_temporary_location("[ many . keys . with . spaces ]");
        const auto res = toml::detail::parse_table_key<toml::type_config>(loc, ctx);
        REQUIRE_UNARY(res.is_ok());

        const auto val = std::get<0>(res.unwrap());
        REQUIRE_UNARY(val.size() == 4);
        REQUIRE_UNARY(val.at(0)  == "many");
        REQUIRE_UNARY(val.at(1)  == "keys");
        REQUIRE_UNARY(val.at(2)  == "with");
        REQUIRE_UNARY(val.at(3)  == "spaces");
    }

    {
        auto loc = toml::detail::make_temporary_location("[ \"one.long.key\" ]");
        const auto res = toml::detail::parse_table_key<toml::type_config>(loc, ctx);
        REQUIRE_UNARY(res.is_ok());

        const auto val = std::get<0>(res.unwrap());
        REQUIRE_UNARY(val.size() == 1);
        REQUIRE_UNARY(val.at(0)  == "one.long.key");
    }
}

TEST_CASE("testing array table keys")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    {
        auto loc = toml::detail::make_temporary_location("[[one-key]]");
        const auto res = toml::detail::parse_array_table_key<toml::type_config>(loc, ctx);
        REQUIRE_UNARY(res.is_ok());

        const auto val = std::get<0>(res.unwrap());
        REQUIRE_UNARY(val.size() == 1);
        REQUIRE_UNARY(val.at(0)  == "one-key");
    }

    {
        auto loc = toml::detail::make_temporary_location("[[many.keys]]");
        const auto res = toml::detail::parse_array_table_key<toml::type_config>(loc, ctx);
        REQUIRE_UNARY(res.is_ok());

        const auto val = std::get<0>(res.unwrap());
        REQUIRE_UNARY(val.size() == 2);
        REQUIRE_UNARY(val.at(0)  == "many");
        REQUIRE_UNARY(val.at(1)  == "keys");
    }

    {
        auto loc = toml::detail::make_temporary_location("[[ many . keys . with . spaces ]]");
        const auto res = toml::detail::parse_array_table_key<toml::type_config>(loc, ctx);
        REQUIRE_UNARY(res.is_ok());

        const auto val = std::get<0>(res.unwrap());
        REQUIRE_UNARY(val.size() == 4);
        REQUIRE_UNARY(val.at(0)  == "many");
        REQUIRE_UNARY(val.at(1)  == "keys");
        REQUIRE_UNARY(val.at(2)  == "with");
        REQUIRE_UNARY(val.at(3)  == "spaces");
    }

    {
        auto loc = toml::detail::make_temporary_location("[[ \"one.long.key\" ]]");
        const auto res = toml::detail::parse_array_table_key<toml::type_config>(loc, ctx);
        REQUIRE_UNARY(res.is_ok());

        const auto val = std::get<0>(res.unwrap());
        REQUIRE_UNARY(val.size() == 1);
        REQUIRE_UNARY(val.at(0)  == "one.long.key");
    }
}


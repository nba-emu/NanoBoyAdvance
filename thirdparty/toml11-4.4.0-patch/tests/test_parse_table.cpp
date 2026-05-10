#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <iostream>
#include "utility.hpp"
#include <toml11/parser.hpp>
#include <toml11/types.hpp>

#include "doctest.h"

TEST_CASE("testing a table")
{
    auto spec = toml::spec::v(1,0,0);
    {
        toml::detail::context<toml::type_config> ctx(spec);
        auto loc = toml::detail::make_temporary_location(R"(a = "foo")");

        toml::value table{toml::table()};
        const auto res = toml::detail::parse_table<toml::type_config>(loc, ctx, table);
        REQUIRE_UNARY(res.is_ok());

        REQUIRE_UNARY(table.is_table());

        CHECK_UNARY(table.as_table().at("a").is_string());
        CHECK_EQ   (table.as_table().at("a").as_string(), "foo");
        CHECK_UNARY(table.at("a").is_string());
        CHECK_EQ   (table.at("a").as_string(), "foo");

        CHECK_EQ(table.as_table_fmt().indent_type,    toml::indent_char::none);
        CHECK_EQ(table.as_table_fmt().name_indent,    0);
        CHECK_EQ(table.as_table_fmt().body_indent,    0);
        CHECK_EQ(table.as_table_fmt().closing_indent, 0);
    }

    {
        toml::detail::context<toml::type_config> ctx(spec);
        auto loc = toml::detail::make_temporary_location("  a = \"foo\"");

        toml::value table{toml::table()};
        const auto res = toml::detail::parse_table<toml::type_config>(loc, ctx, table);

        REQUIRE_UNARY(res.is_ok());

        REQUIRE_UNARY(table.is_table());

        CHECK_UNARY(table.as_table().at("a").is_string());
        CHECK_EQ   (table.as_table().at("a").as_string(), "foo");
        CHECK_UNARY(table.at("a").is_string());
        CHECK_EQ   (table.at("a").as_string(), "foo");

        CHECK_EQ(table.as_table_fmt().indent_type,    toml::indent_char::space);
        CHECK_EQ(table.as_table_fmt().name_indent,    0);
        CHECK_EQ(table.as_table_fmt().body_indent,    2);
        CHECK_EQ(table.as_table_fmt().closing_indent, 0);
    }
    {
        toml::detail::context<toml::type_config> ctx(spec);
        auto loc = toml::detail::make_temporary_location("\ta = \"foo\"");

        toml::value table{toml::table()};
        const auto res = toml::detail::parse_table<toml::type_config>(loc, ctx, table);
        REQUIRE_UNARY(res.is_ok());

        REQUIRE_UNARY(table.is_table());

        CHECK_UNARY(table.as_table().at("a").is_string());
        CHECK_EQ   (table.as_table().at("a").as_string(), "foo");
        CHECK_UNARY(table.at("a").is_string());
        CHECK_EQ   (table.at("a").as_string(), "foo");

        CHECK_EQ(table.as_table_fmt().indent_type,    toml::indent_char::tab);
        CHECK_EQ(table.as_table_fmt().name_indent,    0);
        CHECK_EQ(table.as_table_fmt().body_indent,    1);
        CHECK_EQ(table.as_table_fmt().closing_indent, 0);
    }

    {
        toml::detail::context<toml::type_config> ctx(spec);
        auto loc = toml::detail::make_temporary_location(R"(a = "foo"
b = "bar"
c.c1 = "baz"
c.c2 = "qux"
)");

        toml::value table{toml::table()};
        const auto res = toml::detail::parse_table<toml::type_config>(loc, ctx, table);
        REQUIRE_UNARY(res.is_ok());

        REQUIRE_UNARY(table.is_table());

        CHECK_UNARY(table.at("a").is_string());
        CHECK_EQ   (table.at("a").as_string(), "foo");
        CHECK_UNARY(table.at("b").is_string());
        CHECK_EQ   (table.at("b").as_string(), "bar");
        CHECK_UNARY(table.at("c").at("c1").is_string());
        CHECK_EQ   (table.at("c").at("c1").as_string(), "baz");
        CHECK_UNARY(table.at("c").at("c2").is_string());
        CHECK_EQ   (table.at("c").at("c2").as_string(), "qux");

        CHECK_EQ(table.as_table_fmt().indent_type,    toml::indent_char::none);
        CHECK_EQ(table.as_table_fmt().name_indent,    0);
        CHECK_EQ(table.as_table_fmt().body_indent,    0);
        CHECK_EQ(table.as_table_fmt().closing_indent, 0);
    }


    {
        toml::detail::context<toml::type_config> ctx(spec);
        auto loc = toml::detail::make_temporary_location(R"(a = "foo"
b = "bar"
c.c1 = "baz"
[next.table]
c.c2 = "qux"
)");

        toml::value table{toml::table()};
        const auto res = toml::detail::parse_table<toml::type_config>(loc, ctx, table);
        REQUIRE_UNARY(res.is_ok());

        REQUIRE_UNARY(table.is_table());

        CHECK_UNARY(table.at("a").is_string());
        CHECK_EQ   (table.at("a").as_string(), "foo");
        CHECK_UNARY(table.at("b").is_string());
        CHECK_EQ   (table.at("b").as_string(), "bar");
        CHECK_UNARY(table.at("c").at("c1").is_string());
        CHECK_EQ   (table.at("c").at("c1").as_string(), "baz");
        CHECK_UNARY( ! table.at("c").contains("c2"));

        CHECK_EQ(table.as_table_fmt().indent_type,    toml::indent_char::none);
        CHECK_EQ(table.as_table_fmt().name_indent,    0);
        CHECK_EQ(table.as_table_fmt().body_indent,    0);
        CHECK_EQ(table.as_table_fmt().closing_indent, 0);
    }

    {
        toml::detail::context<toml::type_config> ctx(spec);
        auto loc = toml::detail::make_temporary_location(R"(
    a = "foo"
    b = "bar"
    c.c1 = "baz"
  [next.table]
    c.c2 = "qux"
)");

        toml::value table{toml::table()};
        const auto res = toml::detail::parse_table<toml::type_config>(loc, ctx, table);
        REQUIRE_UNARY(res.is_ok());

        REQUIRE_UNARY(table.is_table());

        CHECK_UNARY(table.at("a").is_string());
        CHECK_EQ   (table.at("a").as_string(), "foo");
        CHECK_UNARY(table.at("b").is_string());
        CHECK_EQ   (table.at("b").as_string(), "bar");
        CHECK_UNARY(table.at("c").at("c1").is_string());
        CHECK_EQ   (table.at("c").at("c1").as_string(), "baz");
        CHECK_UNARY( ! table.at("c").contains("c2"));

        CHECK_EQ(table.as_table_fmt().indent_type,    toml::indent_char::space);
        CHECK_EQ(table.as_table_fmt().name_indent,    0);
        CHECK_EQ(table.as_table_fmt().body_indent,    4);
        CHECK_EQ(table.as_table_fmt().closing_indent, 0);

    }
}

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/parser.hpp>
#include <toml11/serializer.hpp>
#include <toml11/types.hpp>

TEST_CASE("testing multiline table")
{
    const auto table_fmt = [](toml::indent_char c, std::int32_t body, std::int32_t name) {
        toml::table_format_info fmt;
        fmt.fmt = toml::table_format::multiline;
        fmt.indent_type    = c;
        fmt.body_indent    = body;
        fmt.name_indent    = name;
        fmt.closing_indent = 0;
        return fmt;
    };

    toml::value v(toml::table{
            {"a", 42},
            {"b", 3.14},
            {"c", "foobar"},
        });

    v.as_table_fmt() = table_fmt(toml::indent_char::space, 0, 0);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::space, 0, 2);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::space, 2, 0);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::space, 2, 2);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));


    v.as_table_fmt() = table_fmt(toml::indent_char::tab, 0, 0);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::tab, 0, 2);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::tab, 2, 0);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::tab, 2, 2);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));
}

TEST_CASE("testing multiline table")
{
    const auto table_fmt = [](toml::indent_char c, std::int32_t body, std::int32_t name) {
        toml::table_format_info fmt;
        fmt.fmt = toml::table_format::dotted;
        fmt.indent_type    = c;
        fmt.body_indent    = body;
        fmt.name_indent    = name;
        fmt.closing_indent = 0;
        return fmt;
    };

    {
        toml::value v(toml::table{});
        v["a"]["b"]["c"] = "foobar";
        CHECK_EQ(v, toml::parse_str(toml::format(v)));

        v.at("a").as_table_fmt() = table_fmt(toml::indent_char::space, 0, 0);
        CHECK_EQ(v, toml::parse_str(toml::format(v)));

        v.at("a").at("b").as_table_fmt() = table_fmt(toml::indent_char::space, 0, 0);
        CHECK_EQ(v, toml::parse_str(toml::format(v)));

        v.at("a").as_table_fmt().fmt = toml::table_format::multiline;
        CHECK_EQ(v, toml::parse_str(toml::format(v)));
    }
}

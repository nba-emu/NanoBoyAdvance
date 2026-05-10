#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <iostream>
#include "utility.hpp"
#include <toml11/parser.hpp>
#include <toml11/types.hpp>

#include "doctest.h"

TEST_CASE("testing an inline table v1.0")
{
    {
        // no multiline, no trailing comma
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));

        const auto make_format = [](toml::table_format ty) {
            toml::table_format_info fmt;
            fmt.fmt = ty;
            fmt.indent_type = toml::indent_char::none;
            return fmt;
        };

        toml11_test_parse_success<toml::value_t::table>("{}",                 (toml::table{}),                                         comments(), make_format(toml::table_format::oneline), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a = 1}",            (toml::table{{"a", 1}}),                                 comments(), make_format(toml::table_format::oneline), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a = 1, b = 2}",     (toml::table{{"a", 1}, {"b", 2}}),                       comments(), make_format(toml::table_format::oneline), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a.b = 1}",          (toml::table{{ "a", toml::table{{"b", 1}} }}),           comments(), make_format(toml::table_format::oneline), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a.b = 1, a.c = 2}", (toml::table{{ "a", toml::table{{"b", 1}, {"c", 2}} }}), comments(), make_format(toml::table_format::oneline), ctx);
    }

    // invalids
    {
        using toml::detail::parse_inline_table;
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ 1, 2 }",               ctx); // no key
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a = 1, b = 2, }",      ctx); // trailing comma
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a = 1, b = 2",         ctx); // no closing bracket
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{",                      ctx); // no closing bracket
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a = 1, b = 2\n a = b", ctx); // no closing bracket
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a.b = 1, a.b.c = 2}",  ctx); // a.b is not a table
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a = 1 # comment\n}",   ctx); // newline not allowed
    }
}

TEST_CASE("testing an inline table v1.0 + trailing comma")
{
    {
        // no multiline, no trailing comma
        auto spec = toml::spec::v(1,0,0);
        spec.v1_1_0_allow_trailing_comma_in_inline_tables = true;
        toml::detail::context<toml::type_config> ctx(spec);

        const auto make_format = [](toml::table_format ty) {
            toml::table_format_info fmt;
            fmt.fmt = ty;
            fmt.indent_type = toml::indent_char::none;
            return fmt;
        };

        toml11_test_parse_success<toml::value_t::table>("{}",                 (toml::table{}),                                         comments(), make_format(toml::table_format::oneline), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a = 1}",            (toml::table{{"a", 1}}),                                 comments(), make_format(toml::table_format::oneline), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a = 1,}",           (toml::table{{"a", 1}}),                                 comments(), make_format(toml::table_format::oneline), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a = 1, b = 2,}",    (toml::table{{"a", 1}, {"b", 2}}),                       comments(), make_format(toml::table_format::oneline), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a.b = 1}",          (toml::table{{ "a", toml::table{{"b", 1}} }}),           comments(), make_format(toml::table_format::oneline), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a.b = 1, a.c = 2}", (toml::table{{ "a", toml::table{{"b", 1}, {"c", 2}} }}), comments(), make_format(toml::table_format::oneline), ctx);
    }

    // invalids
    {
        using toml::detail::parse_inline_table;

        auto spec = toml::spec::v(1,0,0);
        spec.v1_1_0_allow_trailing_comma_in_inline_tables = true;
        toml::detail::context<toml::type_config> ctx(spec);

        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ 1, 2 }",               ctx);
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a = 1, b = 2",         ctx);
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{",                      ctx);
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a = 1, b = 2\n a = b", ctx);
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a.b = 1, a.b.c = 2}",  ctx);
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a = 1 # comment\n}",   ctx);
    }
}

TEST_CASE("testing an inline table v1.1")
{
    {
        // w/ multiline, w/ trailing comma
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1,1,0));

        const auto make_format = [](toml::table_format ty,
            const toml::indent_char ic,
            const std::int32_t n,
            const std::int32_t b,
            const std::int32_t c
        ) {
            toml::table_format_info fmt;
            fmt.fmt = ty;
            fmt.indent_type    = ic;
            fmt.name_indent    = n;
            fmt.body_indent    = b;
            fmt.closing_indent = c;
            return fmt;
        };

        toml11_test_parse_success<toml::value_t::table>("{}",             (toml::table{}),                   comments(), make_format(toml::table_format::oneline, toml::indent_char::none,0,0,0), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a = 1}",        (toml::table{{"a", 1}}),           comments(), make_format(toml::table_format::oneline, toml::indent_char::none,0,0,0), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a = 1, b = 2}", (toml::table{{"a", 1}, {"b", 2}}), comments(), make_format(toml::table_format::oneline, toml::indent_char::none,0,0,0), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a.b = 1}",          (toml::table{{ "a", toml::table{{"b", 1}} }}),           comments(), make_format(toml::table_format::oneline, toml::indent_char::none,0,0,0), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a.b = 1, a.c = 2}", (toml::table{{ "a", toml::table{{"b", 1}, {"c", 2}} }}), comments(), make_format(toml::table_format::oneline, toml::indent_char::none,0,0,0), ctx);

        toml11_test_parse_success<toml::value_t::table>("{\n}",                     (toml::table{}),                   comments(), make_format(toml::table_format::multiline_oneline, toml::indent_char::none,0,0,0), ctx);
        toml11_test_parse_success<toml::value_t::table>("{\na = 1\n}",              (toml::table{{"a", 1}}),           comments(), make_format(toml::table_format::multiline_oneline, toml::indent_char::none,0,0,0), ctx);

        toml11_test_parse_success<toml::value_t::table>("{\n  a = 1, \n  b = 2\n}", (toml::table{{"a", 1}, {"b", 2}}), comments(), make_format(toml::table_format::multiline_oneline, toml::indent_char::space, 0, 2, 0), ctx);

        toml11_test_parse_success<toml::value_t::table>("{\n  a.b = 1\n  }",        (toml::table{{ "a", toml::table{{"b", 1}} }}),           comments(), make_format(toml::table_format::multiline_oneline, toml::indent_char::space, 0, 2, 2), ctx);

        toml11_test_parse_success<toml::value_t::table>("{a.b = 1,\n  a.c = 2}",    (toml::table{{ "a", toml::table{{"b", 1}, {"c", 2}} }}), comments(), make_format(toml::table_format::multiline_oneline, toml::indent_char::space, 0, 2, 0), ctx);

        toml11_test_parse_success<toml::value_t::table>("{# this table is empty.\n}",              (toml::table{}),                                                                        comments(), make_format(toml::table_format::multiline_oneline, toml::indent_char::none,0,0,0), ctx);
        toml11_test_parse_success<toml::value_t::table>("{\na = 1 # com\n}",                       (toml::table{{"a", toml::value(1, {"# com"}) }}),                                       comments(), make_format(toml::table_format::multiline_oneline, toml::indent_char::none,0,0,0), ctx);

        toml11_test_parse_success<toml::value_t::table>("{\n  a = 1, # com-a\n  b = 2 # com-b\n}", (toml::table{{"a", toml::value(1, {"# com-a"}) }, {"b", toml::value(2, {"# com-b"})}}), comments(), make_format(toml::table_format::multiline_oneline, toml::indent_char::space, 0, 2, 0), ctx);
        toml11_test_parse_success<toml::value_t::table>("{# com-a-b\n  a.b = 1\n  }",              (toml::table{{"a", toml::table{{"b", toml::value(1, {"# com-a-b"})}} }}),               comments(), make_format(toml::table_format::multiline_oneline, toml::indent_char::space, 0, 2, 2), ctx);
        toml11_test_parse_success<toml::value_t::table>("{a.b = 1,\n  a.c = 2 # com-a-c\n}",       (toml::table{{"a", toml::table{{"b", 1}, {"c", toml::value(2, {"# com-a-c"})}} }}),     comments(), make_format(toml::table_format::multiline_oneline, toml::indent_char::space, 0, 2, 0), ctx);
    }

    // invalids
    {
        using toml::detail::parse_inline_table;

        auto spec = toml::spec::v(1,1,0);
        toml::detail::context<toml::type_config> ctx(spec);

        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ 1, 2 }",               ctx); // no key
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a = 1, b = 2",         ctx); // no closing bracket
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{",                      ctx); // no closing bracket
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a = 1, b = 2\n a = b", ctx); // no closing bracket
        toml11_test_parse_failure(parse_inline_table<toml::type_config>, "{ a.b = 1, a.b.c = 2}",  ctx); // a.b is not a table
    }
}


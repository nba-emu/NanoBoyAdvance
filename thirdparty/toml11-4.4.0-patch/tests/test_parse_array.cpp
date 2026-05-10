#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <iostream>
#include "utility.hpp"
#include <toml11/parser.hpp>
#include <toml11/types.hpp>

#include "doctest.h"

TEST_CASE("testing an array")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    const auto make_format = [](toml::array_format ty, toml::indent_char ic, std::int32_t i, std::int32_t c) {
        toml::array_format_info fmt;
        fmt.fmt = ty;
        fmt.indent_type = ic;
        fmt.body_indent = i;
        fmt.closing_indent = c;
        return fmt;
    };

    toml11_test_parse_success<toml::value_t::array>("[]",       (toml::array{}),     comments(), make_format(toml::array_format::oneline, toml::indent_char::none, 4, 0), ctx);
    toml11_test_parse_success<toml::value_t::array>("[1]",      (toml::array{1}),    comments(), make_format(toml::array_format::oneline, toml::indent_char::none, 4, 0), ctx);
    toml11_test_parse_success<toml::value_t::array>("[1, 2]",   (toml::array{1, 2}), comments(), make_format(toml::array_format::oneline, toml::indent_char::none, 4, 0), ctx);
    toml11_test_parse_success<toml::value_t::array>("[1, 2, ]", (toml::array{1, 2}), comments(), make_format(toml::array_format::oneline, toml::indent_char::none, 4, 0), ctx);

    toml11_test_parse_success<toml::value_t::array>("[\n  1,\n  2,\n]",   (toml::array{1, 2}), comments(), make_format(toml::array_format::multiline, toml::indent_char::space, 2, 0), ctx);
    toml11_test_parse_success<toml::value_t::array>("[\n  1,\n  2,\n  ]", (toml::array{1, 2}), comments(), make_format(toml::array_format::multiline, toml::indent_char::space, 2, 2), ctx);
    toml11_test_parse_success<toml::value_t::array>("[\n\t1,\n\t2,\n]",   (toml::array{1, 2}), comments(), make_format(toml::array_format::multiline, toml::indent_char::tab,   1, 0), ctx);
    toml11_test_parse_success<toml::value_t::array>("[\n\t1,\n\t2,\n\t]", (toml::array{1, 2}), comments(), make_format(toml::array_format::multiline, toml::indent_char::tab,   1, 1), ctx);

    // comment 3 is before comma, so is a comment for `2`.
    toml11_test_parse_success<toml::value_t::array>(R"([
    1, # comment 1
    2, # comment 2
# comment 3
  # comment 4
# comment 5
])",
        (toml::array{
             toml::value(1, comments("# comment 1")),
             toml::value(2, comments("# comment 2")),
         }), comments(), make_format(toml::array_format::multiline, toml::indent_char::space, 4, 0), ctx);

    // comment 3 is before comma, so is a comment for `2`.
    toml11_test_parse_success<toml::value_t::array>(R"([
    1, # comment 1
    2, # comment 2
# comment 3
  # comment 4
# comment 5
  ])",
        (toml::array{
             toml::value(1, comments("# comment 1")),
             toml::value(2, comments("# comment 2")),
         }), comments(), make_format(toml::array_format::multiline, toml::indent_char::space, 4, 2), ctx);

    // comment 3 is before comma, so is a comment for `2`.
    toml11_test_parse_success<toml::value_t::array>(R"([1 # comment 1
, 2 # comment 2
# comment 3
, # comment 4
# comment 5 (free comment)
])",
        (toml::array{
             toml::value(1, comments("# comment 1")),
             toml::value(2, comments("# comment 2", "# comment 3", "# comment 4")),
         }), comments(), make_format(toml::array_format::multiline, toml::indent_char::none, 4, 0), ctx);


    toml11_test_parse_success<toml::value_t::array>(R"([1 # comment 1
, # comment 2
# comment 3
# ditto

# comment 4
2 # comment 5
# comment 6
, 3
# comment 7
])",
        (toml::array{
             toml::value(1, comments("# comment 1", "# comment 2")),
             toml::value(2, comments("# comment 4", "# comment 5", "# comment 6")),
             toml::value(3, comments("# comment 7")),
         }), comments(), make_format(toml::array_format::multiline, toml::indent_char::none, 4, 0), ctx);
}

TEST_CASE("testing invalid arrays")
{
    using namespace toml::detail;
    {
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
        auto loc = toml::detail::make_temporary_location("[ 1, hoge ]");
        const auto res = parse_array(loc, ctx);
        CHECK_UNARY(res.is_err());

        for(const auto& e : ctx.errors())
        {
            std::cerr << toml::format_error(e) << std::endl;
        }
    }

    {
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
        auto loc = toml::detail::make_temporary_location("[ 1, 2");
        const auto res = parse_array(loc, ctx);
        CHECK_UNARY(res.is_err());
        ctx.report_error(res.unwrap_err());

        for(const auto& e : ctx.errors())
        {
            std::cerr << toml::format_error(e) << std::endl;
        }
    }

    {
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
        auto loc = toml::detail::make_temporary_location("[");
        const auto res = parse_array(loc, ctx);
        CHECK_UNARY(res.is_err());
        ctx.report_error(res.unwrap_err());

        for(const auto& e : ctx.errors())
        {
            std::cerr << toml::format_error(e) << std::endl;
        }
    }

    {
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
        auto loc = toml::detail::make_temporary_location("[ 1, 2, 3\n a = b");
        const auto res = parse_array(loc, ctx);
        CHECK_UNARY(res.is_err());
        ctx.report_error(res.unwrap_err());

        for(const auto& e : ctx.errors())
        {
            std::cerr << toml::format_error(e) << std::endl;
        }
    }
}

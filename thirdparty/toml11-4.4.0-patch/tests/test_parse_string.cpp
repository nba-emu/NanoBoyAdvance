#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"
#include <iostream>

#include <toml11/parser.hpp>
#include <toml11/types.hpp>

TEST_CASE("testing basic string")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    const auto string_fmt = []() {
        // inline basic_string does not use format settings
        toml::string_format_info fmt;
        fmt.fmt = toml::string_format::basic;
        return fmt;
    };

    toml11_test_parse_success<toml::value_t::string>(
        "\"GitHub Cofounder & CEO\\nLikes tater tots and beer.\"",
        "GitHub Cofounder & CEO\nLikes tater tots and beer.", comments(),
        string_fmt(), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "\"192.168.1.1\"",
        "192.168.1.1", comments(),
        string_fmt(), ctx);

    toml11_test_parse_success<toml::value_t::string>(
        "\"\xE4\xB8\xAD\xE5\x9B\xBD\"",
        "\xE4\xB8\xAD\xE5\x9B\xBD", comments(),
        string_fmt(), ctx);

    toml11_test_parse_success<toml::value_t::string>(
        "\"You'll hate me after this - #\"",
        "You'll hate me after this - #", comments(),
        string_fmt(), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "\" And when \\\"'s are in the parse_ml_basic_string, along with # \\\"\"",
        " And when \"'s are in the parse_ml_basic_string, along with # \"", comments(),
        string_fmt(), ctx);

    // -------------------------------------------------------------------------
    // the same test cases, but with parse_string.

    toml11_test_parse_success<toml::value_t::string>(
        "\"GitHub Cofounder & CEO\\nLikes tater tots and beer.\"",
        "GitHub Cofounder & CEO\nLikes tater tots and beer.", comments(),
        string_fmt(), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "\"192.168.1.1\"",
        "192.168.1.1", comments(),
        string_fmt(), ctx);

    toml11_test_parse_success<toml::value_t::string>(
        "\"\xE4\xB8\xAD\xE5\x9B\xBD\"",
        "\xE4\xB8\xAD\xE5\x9B\xBD", comments(),
        string_fmt(), ctx);

    toml11_test_parse_success<toml::value_t::string>(
        "\"You'll hate me after this - #\"",
        "You'll hate me after this - #", comments(),
        string_fmt(), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "\" And when \\\"'s are in the parse_ml_basic_string, along with # \\\"\"",
        " And when \"'s are in the parse_ml_basic_string, along with # \"", comments(),
        string_fmt(), ctx);
}

TEST_CASE("testing multiline basic string")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    const auto string_fmt = [](bool nl) {
        toml::string_format_info fmt;
        fmt.fmt = toml::string_format::multiline_basic;
        fmt.start_with_newline = nl;
        return fmt;
    };

    toml11_test_parse_success<toml::value_t::string>(
        //       0         1
        //       01234567890123456
        "\"\"\"\nThe quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\"\"\"",
        "The quick brown fox jumps over the lazy dog.", comments(),
        string_fmt(true), ctx
    );

    toml11_test_parse_success<toml::value_t::string>(
        //         0         1
        //         012345678901234567
        "\"\"\"\\\n  The quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\\\n  \"\"\"",
        "The quick brown fox jumps over the lazy dog.", comments(),
        string_fmt(false), ctx
    );

    toml11_test_parse_success<toml::value_t::string>(
        "\"\"\"Here are two quotation marks: \"\". Simple enough.\"\"\"",
        "Here are two quotation marks: \"\". Simple enough.", comments(),
        string_fmt(false), ctx
    );

    toml11_test_parse_success<toml::value_t::string>(
        "\"\"\"Here are three quotation marks: \"\"\\\".\"\"\"",
        "Here are three quotation marks: \"\"\".", comments(),
        string_fmt(false), ctx
    );

    toml11_test_parse_success<toml::value_t::string>(
        "\"\"\"Here are fifteen quotation marks: \"\"\\\"\"\"\\\"\"\"\\\"\"\"\\\"\"\"\\\".\"\"\"",
        "Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\".", comments(),
        string_fmt(false), ctx
    );

    toml11_test_parse_success<toml::value_t::string>(
        "\"\"\"\"This,\" she said, \"is just a pointless statement.\"\"\"\"",
        "\"This,\" she said, \"is just a pointless statement.\"", comments(),
        string_fmt(false), ctx
    );

    // -------------------------------------------------------------------------
    // the same test cases, but with parse_string.

    toml11_test_parse_success<toml::value_t::string>(
        //       0         1
        //       01234567890123456
        "\"\"\"\nThe quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\"\"\"",
        "The quick brown fox jumps over the lazy dog.", comments(),
        string_fmt(true), ctx
    );

    toml11_test_parse_success<toml::value_t::string>(
        //         0         1
        //         012345678901234567
        "\"\"\"\\\n  The quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\\\n  \"\"\"",
        "The quick brown fox jumps over the lazy dog.", comments(),
        string_fmt(false), ctx
    );

    toml11_test_parse_success<toml::value_t::string>(
        "\"\"\"Here are two quotation marks: \"\". Simple enough.\"\"\"",
        "Here are two quotation marks: \"\". Simple enough.", comments(),
        string_fmt(false), ctx
    );

    toml11_test_parse_success<toml::value_t::string>(
        "\"\"\"Here are three quotation marks: \"\"\\\".\"\"\"",
        "Here are three quotation marks: \"\"\".", comments(),
        string_fmt(false), ctx
    );

    toml11_test_parse_success<toml::value_t::string>(
        "\"\"\"Here are fifteen quotation marks: \"\"\\\"\"\"\\\"\"\"\\\"\"\"\\\"\"\"\\\".\"\"\"",
        "Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\".", comments(),
        string_fmt(false), ctx
    );

    toml11_test_parse_success<toml::value_t::string>(
        "\"\"\"\"This,\" she said, \"is just a pointless statement.\"\"\"\"",
        "\"This,\" she said, \"is just a pointless statement.\"", comments(),
        string_fmt(false), ctx
    );
}

TEST_CASE("testing literal_string")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    const auto string_fmt = []() {
        // inline basic_string does not use format settings
        toml::string_format_info fmt;
        fmt.fmt = toml::string_format::literal;
        return fmt;
    };

    toml11_test_parse_success<toml::value_t::string>(
        "'C:\\Users\\nodejs\\templates'",
        "C:\\Users\\nodejs\\templates", comments(),
        string_fmt(), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "'\\\\ServerX\\admin$\\system32\\'",
        "\\\\ServerX\\admin$\\system32\\", comments(),
        string_fmt(), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "'Tom \"Dubs\" Preston-Werner'",
        "Tom \"Dubs\" Preston-Werner", comments(),
        string_fmt(), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "'<\\i\\c*\\s*>'",
        "<\\i\\c*\\s*>", comments(),
        string_fmt(), ctx);

    // -------------------------------------------------------------------------
    // the same test cases, but with parse_string.

    toml11_test_parse_success<toml::value_t::string>(
        "'C:\\Users\\nodejs\\templates'",
        "C:\\Users\\nodejs\\templates", comments(),
        string_fmt(), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "'\\\\ServerX\\admin$\\system32\\'",
        "\\\\ServerX\\admin$\\system32\\", comments(),
        string_fmt(), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "'Tom \"Dubs\" Preston-Werner'",
        "Tom \"Dubs\" Preston-Werner", comments(),
        string_fmt(), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "'<\\i\\c*\\s*>'",
        "<\\i\\c*\\s*>", comments(),
        string_fmt(), ctx);
}

TEST_CASE("testing ml_literal_string")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    const auto string_fmt = [](bool nl) {
        toml::string_format_info fmt;
        fmt.fmt = toml::string_format::multiline_literal;
        fmt.start_with_newline = nl;
        return fmt;
    };

    toml11_test_parse_success<toml::value_t::string>(
        "'''I [dw]on't need \\d{2} apples'''",
        "I [dw]on't need \\d{2} apples", comments(),
        string_fmt(false), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "'''\nThe first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n'''",
        "The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n", comments(),
        string_fmt(true), ctx);

    toml11_test_parse_success<toml::value_t::string>(
        "''''That's still pointless', she said.'''",
        "'That's still pointless', she said.", comments(),
        string_fmt(false), ctx);

    toml11_test_parse_success<toml::value_t::string>(
        "'''Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\".'''",
        "Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\".", comments(),
        string_fmt(false), ctx);

    toml11_test_parse_success<toml::value_t::string>(
        "''''This,' she said, 'is just a pointless statement.''''",
        "'This,' she said, 'is just a pointless statement.'", comments(),
        string_fmt(false), ctx);

    // -------------------------------------------------------------------------
    // the same test cases, but with parse_string.

    toml11_test_parse_success<toml::value_t::string>(
        "'''I [dw]on't need \\d{2} apples'''",
        "I [dw]on't need \\d{2} apples", comments(),
        string_fmt(false), ctx);
    toml11_test_parse_success<toml::value_t::string>(
        "'''\nThe first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n'''",
        "The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n", comments(),
        string_fmt(true), ctx);

    toml11_test_parse_success<toml::value_t::string>(
        "''''That's still pointless', she said.'''",
        "'That's still pointless', she said.", comments(),
        string_fmt(false), ctx);

    toml11_test_parse_success<toml::value_t::string>(
        "'''Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\".'''",
        "Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\".", comments(),
        string_fmt(false), ctx);

    toml11_test_parse_success<toml::value_t::string>(
        "''''This,' she said, 'is just a pointless statement.''''",
        "'This,' she said, 'is just a pointless statement.'", comments(),
        string_fmt(false), ctx);
}

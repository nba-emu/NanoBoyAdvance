#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/syntax.hpp>

TEST_CASE("testing string")
{
    const auto string            = toml::detail::syntax::string(toml::spec::v(1,0,0));
    const auto ml_basic_string   = toml::detail::syntax::ml_basic_string(toml::spec::v(1,0,0));
    const auto ml_literal_string = toml::detail::syntax::ml_literal_string(toml::spec::v(1,0,0));

    test_scan_success(string,
            "\"The quick brown fox jumps over the lazy dog\"",
            "\"The quick brown fox jumps over the lazy dog\"");
    test_scan_success(string,
            "\'The quick brown fox jumps over the lazy dog\'",
            "\'The quick brown fox jumps over the lazy dog\'");
    test_scan_success(ml_basic_string,
            "\"\"\"The quick brown fox jumps over the lazy dog\"\"\"",
            "\"\"\"The quick brown fox jumps over the lazy dog\"\"\"");
    test_scan_success(ml_basic_string,
            "\"\"\"The quick brown fox \\\njumps over the lazy dog\"\"\"",
            "\"\"\"The quick brown fox \\\njumps over the lazy dog\"\"\"");
    test_scan_success(ml_literal_string,
            "'''The quick brown fox jumps over the lazy dog'''",
            "'''The quick brown fox jumps over the lazy dog'''");
    test_scan_success(ml_literal_string,
            "'''The quick brown fox \njumps over the lazy dog'''",
            "'''The quick brown fox \njumps over the lazy dog'''");
}

TEST_CASE("testing basic_string")
{
    const auto string = toml::detail::syntax::string(toml::spec::v(1,0,0));

    test_scan_success(string,
            "\"GitHub Cofounder & CEO\\nLikes tater tots and beer.\"",
            "\"GitHub Cofounder & CEO\\nLikes tater tots and beer.\"");
    test_scan_success(string,
            "\"192.168.1.1\"",
            "\"192.168.1.1\"");

    test_scan_success(string,
            "\"\xE4\xB8\xAD\xE5\x9B\xBD\"",  // UTF-8 string (means "China" in
            "\"\xE4\xB8\xAD\xE5\x9B\xBD\""); // Chinese characters)

    test_scan_success(string,
            "\"You'll hate me after this - #\"",
            "\"You'll hate me after this - #\"");
    test_scan_success(string,
            "\" And when \\\"'s are in the string, along with # \\\"\"",
            "\" And when \\\"'s are in the string, along with # \\\"\"");
}

TEST_CASE("testing ml_basic_string")
{
    const auto string = toml::detail::syntax::string(toml::spec::v(1,0,0));

    test_scan_success(string,
        "\"\"\"\nThe quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\"\"\"",
        "\"\"\"\nThe quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\"\"\"");
    test_scan_success(string,
        "\"\"\"\\\n  The quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\\\n  \"\"\"",
        "\"\"\"\\\n  The quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\\\n  \"\"\"");

    test_scan_success(string,
        "\"\"\"Here are two quotation marks: \"\". Simple enough.\"\"\"",
        "\"\"\"Here are two quotation marks: \"\". Simple enough.\"\"\"");

    test_scan_success(string,
        "\"\"\"Here are three quotation marks: \"\"\\\".\"\"\"",
        "\"\"\"Here are three quotation marks: \"\"\\\".\"\"\"");

    test_scan_success(string,
        "\"\"\"Here are fifteen quotation marks: \"\"\\\"\"\"\\\"\"\"\\\"\"\"\\\"\"\"\\\".\"\"\"",
        "\"\"\"Here are fifteen quotation marks: \"\"\\\"\"\"\\\"\"\"\\\"\"\"\\\"\"\"\\\".\"\"\"");

    test_scan_success(string,
        "\"\"\"\"This,\" she said, \"is just a pointless statement.\"\"\"\"",
        "\"\"\"\"This,\" she said, \"is just a pointless statement.\"\"\"\"");
}

TEST_CASE("testing literal_string")
{
    const auto string = toml::detail::syntax::string(toml::spec::v(1,0,0));

    test_scan_success(string,
        "'C:\\Users\\nodejs\\templates'",
        "'C:\\Users\\nodejs\\templates'");
    test_scan_success(string,
        "'\\\\ServerX\\admin$\\system32\\'",
        "'\\\\ServerX\\admin$\\system32\\'");
    test_scan_success(string,
        "'Tom \"Dubs\" Preston-Werner'",
        "'Tom \"Dubs\" Preston-Werner'");
    test_scan_success(string,
        "'<\\i\\c*\\s*>'",
        "'<\\i\\c*\\s*>'");
}

TEST_CASE("testing ml_literal_string")
{
    const auto string = toml::detail::syntax::string(toml::spec::v(1,0,0));

    test_scan_success(string,
        "'''I [dw]on't need \\d{2} apples'''",
        "'''I [dw]on't need \\d{2} apples'''");
    test_scan_success(string,
        "'''\nThe first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n'''",
        "'''\nThe first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n'''");

    test_scan_success(string,
        "''''That's still pointless', she said.'''",
        "''''That's still pointless', she said.'''");

    test_scan_success(string,
        "'''Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\".'''",
        "'''Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\".'''");

    test_scan_success(string,
        "''''This,' she said, 'is just a pointless statement.''''",
        "''''This,' she said, 'is just a pointless statement.''''");
}

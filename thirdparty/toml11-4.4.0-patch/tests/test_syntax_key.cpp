#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/syntax.hpp>

TEST_CASE("testing bare_key")
{
    const auto scanner = toml::detail::syntax::key(toml::spec::v(1,0,0));

    test_scan_success(scanner, "barekey",  "barekey");
    test_scan_success(scanner, "bare-key", "bare-key");
    test_scan_success(scanner, "bare_key", "bare_key");
    test_scan_success(scanner, "1234",     "1234");
}

TEST_CASE("testing quoted_key")
{
    const auto scanner = toml::detail::syntax::key(toml::spec::v(1,0,0));

    test_scan_success(scanner, "\"127.0.0.1\"", "\"127.0.0.1\"");
    test_scan_success(scanner, "\"character encoding\"", "\"character encoding\"");

    // UTF-8 codepoint of characters that looks like "key" written upside down
    test_scan_success(scanner, "\"\xCA\x8E\xC7\x9D\xCA\x9E\"",
                                    "\"\xCA\x8E\xC7\x9D\xCA\x9E\"");

    test_scan_success(scanner, "'key2'", "'key2'");
    test_scan_success(scanner, "'quoted \"value\"'", "'quoted \"value\"'");
}

TEST_CASE("testing dotted_key")
{
    const auto scanner = toml::detail::syntax::key(toml::spec::v(1,0,0));

    test_scan_success(scanner, "physical.color", "physical.color");
    test_scan_success(scanner, "physical.shape", "physical.shape");
    test_scan_success(scanner, "x.y", "x.y");
    test_scan_success(scanner, "x . y", "x . y");
    test_scan_success(scanner, "x.y.z", "x.y.z");
    test_scan_success(scanner, "x. y .z", "x. y .z");
    test_scan_success(scanner, "x .y. z", "x .y. z");
    test_scan_success(scanner, "x . y . z", "x . y . z");
    test_scan_success(scanner, "x.y.z.w", "x.y.z.w");
    test_scan_success(scanner, "x. y .z. w", "x. y .z. w");
    test_scan_success(scanner, "x . y . z . w", "x . y . z . w");
    test_scan_success(scanner, "site.\"google.com\"", "site.\"google.com\"");
}

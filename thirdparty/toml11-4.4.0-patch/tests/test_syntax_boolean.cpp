#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/syntax.hpp>

TEST_CASE("testing boolean should success")
{
    const auto scanner = toml::detail::syntax::boolean(toml::spec::v(1,0,0));

    test_scan_success(scanner, "true", "true");
    test_scan_success(scanner, "false", "false");
    test_scan_success(scanner, "true  # comment", "true");
    test_scan_success(scanner, "false # comment", "false");
}

TEST_CASE("testing boolean should fail")
{
    const auto scanner = toml::detail::syntax::boolean(toml::spec::v(1,0,0));

    test_scan_failure(scanner, "TRUE");
    test_scan_failure(scanner, "FALSE");
    test_scan_failure(scanner, "True");
    test_scan_failure(scanner, "False");
    test_scan_failure(scanner, "T");
    test_scan_failure(scanner, "F");
}

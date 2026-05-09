#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/syntax.hpp>

TEST_CASE("testing fractional_valid")
{
    const auto floating = toml::detail::syntax::floating(toml::spec::v(1,0,0));
    test_scan_success(floating, "1.0",               "1.0"              );
    test_scan_success(floating, "0.1",               "0.1"              );
    test_scan_success(floating, "0.001",             "0.001"            );
    test_scan_success(floating, "0.100",             "0.100"            );
    test_scan_success(floating, "+3.14",             "+3.14"            );
    test_scan_success(floating, "-3.14",             "-3.14"            );
    test_scan_success(floating, "3.1415_9265_3589",  "3.1415_9265_3589" );
    test_scan_success(floating, "+3.1415_9265_3589", "+3.1415_9265_3589");
    test_scan_success(floating, "-3.1415_9265_3589", "-3.1415_9265_3589");
    test_scan_success(floating, "123_456.789",       "123_456.789"      );
    test_scan_success(floating, "+123_456.789",      "+123_456.789"     );
    test_scan_success(floating, "-123_456.789",      "-123_456.789"     );
}

TEST_CASE("testing fractional_invalid")
{
    const auto floating = toml::detail::syntax::floating(toml::spec::v(1,0,0));
    test_scan_failure(floating, "0.");
    test_scan_failure(floating, ".0");
    test_scan_failure(floating, "01.0");
    test_scan_failure(floating, "3,14");
    test_scan_failure(floating, "+-1.0");
    test_scan_failure(floating, "1._0");
}

TEST_CASE("testing exponential_valid")
{
    const auto floating = toml::detail::syntax::floating(toml::spec::v(1,0,0));
    test_scan_success(floating, "1e10",       "1e10");
    test_scan_success(floating, "1e+10",      "1e+10");
    test_scan_success(floating, "1e-10",      "1e-10");
    test_scan_success(floating, "+1e10",      "+1e10");
    test_scan_success(floating, "+1e+10",     "+1e+10");
    test_scan_success(floating, "+1e-10",     "+1e-10");
    test_scan_success(floating, "-1e10",      "-1e10");
    test_scan_success(floating, "-1e+10",     "-1e+10");
    test_scan_success(floating, "-1e-10",     "-1e-10");
    test_scan_success(floating, "123e-10",    "123e-10");
    test_scan_success(floating, "1E10",       "1E10");
    test_scan_success(floating, "1E+10",      "1E+10");
    test_scan_success(floating, "1E-10",      "1E-10");
    test_scan_success(floating, "123E-10",    "123E-10");
    test_scan_success(floating, "1_2_3E-10",  "1_2_3E-10");
    test_scan_success(floating, "1_2_3E-1_0", "1_2_3E-1_0");

    test_scan_success(floating, "1_2_3E-01",  "1_2_3E-01");
    test_scan_success(floating, "1_2_3E-0_1", "1_2_3E-0_1");
}

TEST_CASE("testing exponential_invalid")
{
    const auto floating = toml::detail::syntax::floating(toml::spec::v(1,0,0));
    // accept partially
    test_scan_success(floating, "1e1E0", "1e1");
    test_scan_success(floating, "1E1e0", "1E1");
}

TEST_CASE("testing both_valid")
{
    const auto floating = toml::detail::syntax::floating(toml::spec::v(1,0,0));
    test_scan_success(floating, "6.02e23",          "6.02e23");
    test_scan_success(floating, "6.02e+23",         "6.02e+23");
    test_scan_success(floating, "1.112_650_06e-17", "1.112_650_06e-17");

    test_scan_success(floating, "1.0e-07",          "1.0e-07");
}

TEST_CASE("testing both_invalid")
{
    const auto floating = toml::detail::syntax::floating(toml::spec::v(1,0,0));
    test_scan_failure(floating, "01e1.0");
    // accept partially
    test_scan_success(floating, "1e1.0",  "1e1");

    test_scan_success(floating, "1.0e_01",  "1.0");
    test_scan_success(floating, "1.0e0__1", "1.0e0");
}

TEST_CASE("testing special_floating_point")
{
    const auto floating = toml::detail::syntax::floating(toml::spec::v(1,0,0));
    test_scan_success(floating,  "inf",  "inf");
    test_scan_success(floating, "+inf", "+inf");
    test_scan_success(floating, "-inf", "-inf");

    test_scan_success(floating,  "nan",  "nan");
    test_scan_success(floating, "+nan", "+nan");
    test_scan_success(floating, "-nan", "-nan");
}

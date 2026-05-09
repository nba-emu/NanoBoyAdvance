#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/syntax.hpp>

TEST_CASE("testing offset_datetime")
{
    {
        const auto scanner = toml::detail::syntax::offset_datetime(toml::spec::v(1,0,0));

        test_scan_success(scanner,
                "1979-05-27T07:32:00Z",
                "1979-05-27T07:32:00Z");
        test_scan_success(scanner,
                "1979-05-27T07:32:00-07:00",
                "1979-05-27T07:32:00-07:00");
        test_scan_success(scanner,
                "1979-05-27T07:32:00.999999-07:00",
                "1979-05-27T07:32:00.999999-07:00");

        test_scan_success(scanner,
                "1979-05-27 07:32:00Z",
                "1979-05-27 07:32:00Z");
        test_scan_success(scanner,
                "1979-05-27 07:32:00-07:00",
                "1979-05-27 07:32:00-07:00");
        test_scan_success(scanner,
                "1979-05-27 07:32:00.999999-07:00",
                "1979-05-27 07:32:00.999999-07:00");
    }
    {
        auto spec = toml::spec::v(1,0,0);
        spec.v1_1_0_make_seconds_optional = true;
        const auto scanner = toml::detail::syntax::offset_datetime(spec);

        test_scan_success(scanner,
                "1979-05-27T07:32Z",
                "1979-05-27T07:32Z");

        test_scan_success(scanner,
                "1979-05-27T07:32:00Z",
                "1979-05-27T07:32:00Z");
        test_scan_success(scanner,
                "1979-05-27T07:32:00-07:00",
                "1979-05-27T07:32:00-07:00");
        test_scan_success(scanner,
                "1979-05-27T07:32:00.999999-07:00",
                "1979-05-27T07:32:00.999999-07:00");

        test_scan_success(scanner,
                "1979-05-27 07:32Z",
                "1979-05-27 07:32Z");
        test_scan_success(scanner,
                "1979-05-27 07:32:00Z",
                "1979-05-27 07:32:00Z");
        test_scan_success(scanner,
                "1979-05-27 07:32:00-07:00",
                "1979-05-27 07:32:00-07:00");
        test_scan_success(scanner,
                "1979-05-27 07:32:00.999999-07:00",
                "1979-05-27 07:32:00.999999-07:00");
    }

}

TEST_CASE("testing local_datetime")
{
    {
        const auto scanner = toml::detail::syntax::local_datetime(toml::spec::v(1,0,0));

        test_scan_success(scanner,
                "1979-05-27T07:32:00",
                "1979-05-27T07:32:00");
        test_scan_success(scanner,
                "1979-05-27T07:32:00.999999",
                "1979-05-27T07:32:00.999999");

        test_scan_success(scanner,
                "1979-05-27 07:32:00",
                "1979-05-27 07:32:00");
        test_scan_success(scanner,
                "1979-05-27 07:32:00.999999",
                "1979-05-27 07:32:00.999999");
    }
    {
        auto spec = toml::spec::v(1,0,0);
        spec.v1_1_0_make_seconds_optional = true;
        const auto scanner = toml::detail::syntax::local_datetime(spec);

        test_scan_success(scanner,
                "1979-05-27T07:32",
                "1979-05-27T07:32");
        test_scan_success(scanner,
                "1979-05-27T07:32:00",
                "1979-05-27T07:32:00");
        test_scan_success(scanner,
                "1979-05-27T07:32:00.999999",
                "1979-05-27T07:32:00.999999");

        test_scan_success(scanner,
                "1979-05-27 07:32:00",
                "1979-05-27 07:32:00");
        test_scan_success(scanner,
                "1979-05-27 07:32:00.999999",
                "1979-05-27 07:32:00.999999");
    }
}

TEST_CASE("testing local_date")
{
    const auto scanner = toml::detail::syntax::local_date(toml::spec::v(1,0,0));
    test_scan_success(scanner, "1979-05-27", "1979-05-27");
}
TEST_CASE("testing local_time")
{
    {
        const auto scanner = toml::detail::syntax::local_time(toml::spec::v(1,0,0));

        test_scan_success(scanner, "07:32:00", "07:32:00");
        test_scan_success(scanner, "07:32:00.999999", "07:32:00.999999");
    }

    {
        auto spec = toml::spec::v(1,0,0);
        spec.v1_1_0_make_seconds_optional = true;

        const auto scanner = toml::detail::syntax::local_time(spec);

        test_scan_success(scanner, "07:32",    "07:32");
        test_scan_success(scanner, "07:32:00", "07:32:00");
        test_scan_success(scanner, "07:32:00.999999", "07:32:00.999999");
    }

}

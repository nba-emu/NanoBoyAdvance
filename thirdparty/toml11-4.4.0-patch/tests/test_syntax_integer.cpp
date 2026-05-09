#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/syntax.hpp>

TEST_CASE("testing decimal_correct")
{
    const auto integer = toml::detail::syntax::integer(toml::spec::v(1,0,0));

    test_scan_success(integer, "1234",        "1234"       );
    test_scan_success(integer, "+1234",       "+1234"      );
    test_scan_success(integer, "-1234",       "-1234"      );
    test_scan_success(integer, "0",           "0"          );
    test_scan_success(integer, "1_2_3_4",     "1_2_3_4"    );
    test_scan_success(integer, "+1_2_3_4",    "+1_2_3_4"   );
    test_scan_success(integer, "-1_2_3_4",    "-1_2_3_4"   );
    test_scan_success(integer, "123_456_789", "123_456_789");
}

TEST_CASE("testing decimal_invalid")
{
    const auto integer = toml::detail::syntax::integer(toml::spec::v(1,0,0));

    test_scan_success(integer, "123+45",  "123");
    test_scan_success(integer, "123-45",  "123");
    test_scan_success(integer, "01234",   "0");
    test_scan_success(integer, "123__45", "123");

    test_scan_failure(integer, "_1234");
}

TEST_CASE("testing hex_correct")
{
    const auto integer = toml::detail::syntax::integer(toml::spec::v(1,0,0));

    test_scan_success(integer, "0xDEADBEEF",  "0xDEADBEEF" );
    test_scan_success(integer, "0xdeadbeef",  "0xdeadbeef" );
    test_scan_success(integer, "0xDEADbeef",  "0xDEADbeef" );
    test_scan_success(integer, "0xDEAD_BEEF", "0xDEAD_BEEF");
    test_scan_success(integer, "0xdead_beef", "0xdead_beef");
    test_scan_success(integer, "0xdead_BEEF", "0xdead_BEEF");

    test_scan_success(integer, "0xFF",     "0xFF"    );
    test_scan_success(integer, "0x00FF",   "0x00FF"  );
    test_scan_success(integer, "0x0000FF", "0x0000FF");
}

TEST_CASE("testing hex_invalid")
{
    const auto integer = toml::detail::syntax::integer(toml::spec::v(1,0,0));
    const auto hex_int = toml::detail::syntax::hex_int(toml::spec::v(1,0,0));

    test_scan_success(integer, "0xAPPLE",     "0xA");
    test_scan_success(integer, "0xDEAD+BEEF", "0xDEAD");
    test_scan_success(integer, "0xDEAD__BEEF", "0xDEAD");

    test_scan_failure(hex_int, "0x_DEADBEEF");
    test_scan_failure(hex_int, "0x+DEADBEEF");
    test_scan_failure(hex_int, "-0xFF"      );
    test_scan_failure(hex_int, "-0x00FF"    );

    test_scan_success(integer, "0x_DEADBEEF", "0" );
    test_scan_success(integer, "0x+DEADBEEF", "0" );
    test_scan_success(integer, "-0xFF"      , "-0" );
    test_scan_success(integer, "-0x00FF"    , "-0" );
}

TEST_CASE("testing oct_correct")
{
    const auto integer = toml::detail::syntax::integer(toml::spec::v(1,0,0));

    test_scan_success(integer, "0o777",    "0o777"  );
    test_scan_success(integer, "0o7_7_7",  "0o7_7_7");
    test_scan_success(integer, "0o007",    "0o007"  );
}

TEST_CASE("testing oct_invalid")
{
    const auto integer = toml::detail::syntax::integer(toml::spec::v(1,0,0));
    const auto oct_int = toml::detail::syntax::hex_int(toml::spec::v(1,0,0));

    test_scan_success(integer, "0o77+7", "0o77");
    test_scan_success(integer, "0o1__0", "0o1");

    test_scan_failure(oct_int, "0o800" );
    test_scan_failure(oct_int, "-0o777");
    test_scan_failure(oct_int, "0o+777");
    test_scan_failure(oct_int, "0o_10" );

    test_scan_success(integer, "0o800",  "0");
    test_scan_success(integer, "-0o777", "-0");
    test_scan_success(integer, "0o+777", "0");
    test_scan_success(integer, "0o_10",  "0");
}

TEST_CASE("testing bin_correct")
{
    const auto integer = toml::detail::syntax::integer(toml::spec::v(1,0,0));

    test_scan_success(integer, "0b10000",    "0b10000"   );
    test_scan_success(integer, "0b010000",   "0b010000"  );
    test_scan_success(integer, "0b01_00_00", "0b01_00_00");
    test_scan_success(integer, "0b111111",   "0b111111"  );
}

TEST_CASE("testing bin_invalid")
{
    const auto bin_int = toml::detail::syntax::bin_int(toml::spec::v(1,0,0));

    test_scan_success(bin_int, "0b11__11", "0b11");
    test_scan_success(bin_int, "0b11+11" , "0b11");

    test_scan_failure(bin_int, "-0b10000");
    test_scan_failure(bin_int, "0b_1111" );
}

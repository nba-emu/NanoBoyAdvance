
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/parser.hpp>
#include <toml11/types.hpp>

TEST_CASE("testing decimal_value")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    const auto decimal_fmt = [](std::size_t w, std::size_t s) {
        toml::integer_format_info fmt;
        fmt.fmt = toml::integer_format::dec;
        fmt.width = w;
        fmt.spacer = s;
        return fmt;
    };

    toml11_test_parse_success<toml::value_t::integer>(          "0",         0, comments(), decimal_fmt(1, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>(         "+0",         0, comments(), decimal_fmt(2, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>(         "-0",         0, comments(), decimal_fmt(2, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>(       "1234",      1234, comments(), decimal_fmt(4, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>(      "+1234",      1234, comments(), decimal_fmt(5, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>(      "-1234",     -1234, comments(), decimal_fmt(5, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>(          "0",         0, comments(), decimal_fmt(1, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>(    "1_2_3_4",      1234, comments(), decimal_fmt(4, 1), ctx);
    toml11_test_parse_success<toml::value_t::integer>(   "+1_2_3_4",     +1234, comments(), decimal_fmt(5, 1), ctx);
    toml11_test_parse_success<toml::value_t::integer>(   "-1_2_3_4",     -1234, comments(), decimal_fmt(5, 1), ctx);
    toml11_test_parse_success<toml::value_t::integer>("123_456_789", 123456789, comments(), decimal_fmt(9, 3), ctx);
}

TEST_CASE("testing hex_value")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    const auto hex_fmt = [](bool u, std::size_t w, std::size_t s) {
        toml::integer_format_info fmt;
        fmt.fmt = toml::integer_format::hex;
        fmt.uppercase = u;
        fmt.width = w;
        fmt.spacer = s;
        return fmt;
    };

    toml11_test_parse_success<toml::value_t::integer>("0xDEADBEEF",  0xDEADBEEF, comments(), hex_fmt(true,  8, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0xdeadbeef",  0xDEADBEEF, comments(), hex_fmt(false, 8, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0xDEADbeef",  0xDEADBEEF, comments(), hex_fmt(true,  8, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0xDEAD_BEEF", 0xDEADBEEF, comments(), hex_fmt(true,  8, 4), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0xdead_beef", 0xDEADBEEF, comments(), hex_fmt(false, 8, 4), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0xdead_BEEF", 0xDEADBEEF, comments(), hex_fmt(true,  8, 4), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0xFF",        0xFF,       comments(), hex_fmt(true,  2, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0x00FF",      0xFF,       comments(), hex_fmt(true,  4, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0x0000FF",    0xFF,       comments(), hex_fmt(true,  6, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0xff",        0xFF,       comments(), hex_fmt(false, 2, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0x00ff",      0xFF,       comments(), hex_fmt(false, 4, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0x0000ff",    0xFF,       comments(), hex_fmt(false, 6, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0x00",        0,          comments(), hex_fmt(true,  2, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0x12345",     0x12345,    comments(), hex_fmt(true,  5, 0), ctx);
}

TEST_CASE("testing oct_value")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    const auto oct_fmt = [](std::size_t w, std::size_t s) {
        toml::integer_format_info fmt;
        fmt.fmt = toml::integer_format::oct;
        fmt.width = w;
        fmt.spacer = s;
        return fmt;
    };

    toml11_test_parse_success<toml::value_t::integer>("0o777",   64*7+8*7+7, comments(), oct_fmt(3, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0o7_7_7", 64*7+8*7+7, comments(), oct_fmt(3, 1), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0o007",            7, comments(), oct_fmt(3, 0), ctx);
}

TEST_CASE("testing bin_value")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    const auto bin_fmt = [](std::size_t w, std::size_t s) {
        toml::integer_format_info fmt;
        fmt.fmt = toml::integer_format::bin;
        fmt.width = w;
        fmt.spacer = s;
        return fmt;
    };

    toml11_test_parse_success<toml::value_t::integer>("0b10000",    16, comments(), bin_fmt(5, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0b010000",   16, comments(), bin_fmt(6, 0), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0b01_00_00", 16, comments(), bin_fmt(6, 2), ctx);
    toml11_test_parse_success<toml::value_t::integer>("0b111111",   63, comments(), bin_fmt(6, 0), ctx);

    toml11_test_parse_success<toml::value_t::integer>(
        "0b1000_1000_1000_1000_1000_1000_1000_1000_1000_1000_1000_1000_1000_1000_1000",
        0x0888888888888888, comments(), bin_fmt(60, 4), ctx);
    toml11_test_parse_success<toml::value_t::integer>(
        "0b01111111_11111111_11111111_11111111_11111111_11111111_11111111_11111111",
        0x7FFFFFFFFFFFFFFF, comments(), bin_fmt(64, 8), ctx);
    toml11_test_parse_success<toml::value_t::integer>(
        "0b00000000_01111111_11111111_11111111_11111111_11111111_11111111_11111111_11111111",
        0x7FFFFFFFFFFFFFFF, comments(), bin_fmt(72, 8), ctx);
}

TEST_CASE("testing integer_overflow")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));
    {
        auto loc = toml::detail::make_temporary_location("9223372036854775808");
        const auto res = toml::detail::parse_dec_integer(loc, ctx);
        CHECK_UNARY(res.is_err());
    }
    {
        auto loc = toml::detail::make_temporary_location("0x1_00000000_00000000");
        const auto res = toml::detail::parse_hex_integer(loc, ctx);
        CHECK_UNARY(res.is_err());
    }
    {
        auto loc = toml::detail::make_temporary_location("0o1_000_000_000_000_000_000_000");
        const auto res = toml::detail::parse_oct_integer(loc, ctx);
        CHECK_UNARY(res.is_err());
    }
    {
        //                         64       56       48       40       32       24       16        8
        auto loc = toml::detail::make_temporary_location("0b10000000_00000000_00000000_00000000_00000000_00000000_00000000_00000000");
        const auto res = toml::detail::parse_oct_integer(loc, ctx);
        CHECK_UNARY(res.is_err());
    }
}

TEST_CASE("testing decimal_value with suffix extension")
{
    auto spec = toml::spec::v(1, 0, 0);
    spec.ext_num_suffix = true;

    toml::detail::context<toml::type_config> ctx(spec);
    const auto decimal_fmt = [](std::size_t w, std::size_t s, std::string x) {
        toml::integer_format_info fmt;
        fmt.fmt = toml::integer_format::dec;
        fmt.width = w;
        fmt.spacer = s;
        fmt.suffix = std::move(x);
        return fmt;
    };
    toml11_test_parse_success<toml::value_t::integer>(       "1234_μm",      1234, comments(), decimal_fmt(4, 0, "μm"), ctx);
    toml11_test_parse_success<toml::value_t::integer>(      "+1234_μm",      1234, comments(), decimal_fmt(5, 0, "μm"), ctx);
    toml11_test_parse_success<toml::value_t::integer>(      "-1234_μm",     -1234, comments(), decimal_fmt(5, 0, "μm"), ctx);
    toml11_test_parse_success<toml::value_t::integer>(          "0_μm",         0, comments(), decimal_fmt(1, 0, "μm"), ctx);
    toml11_test_parse_success<toml::value_t::integer>(    "1_2_3_4_μm",      1234, comments(), decimal_fmt(4, 1, "μm"), ctx);
    toml11_test_parse_success<toml::value_t::integer>(   "+1_2_3_4_μm",     +1234, comments(), decimal_fmt(5, 1, "μm"), ctx);
    toml11_test_parse_success<toml::value_t::integer>(   "-1_2_3_4_μm",     -1234, comments(), decimal_fmt(5, 1, "μm"), ctx);
    toml11_test_parse_success<toml::value_t::integer>("123_456_789_μm", 123456789, comments(), decimal_fmt(9, 3, "μm"), ctx);
}

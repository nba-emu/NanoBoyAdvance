#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <iostream>

#include <toml11/value.hpp>
#include <toml11/literal.hpp>

TEST_CASE("testing toml value literal")
{
    using namespace toml::literals::toml_literals;
    {
        const toml::value v1 = "true"_toml;
        const toml::value v2 = "false"_toml;

        CHECK_UNARY(v1.is_boolean());
        CHECK_UNARY(v2.is_boolean());
        CHECK_EQ(v1.as_boolean(), true);
        CHECK_EQ(v2.as_boolean(), false);
    }
    {
        const toml::value v1 = "123_456"_toml;
        const toml::value v2 = "0b0010"_toml;
        const toml::value v3 = "0xDEADBEEF"_toml;

        CHECK_UNARY(v1.is_integer());
        CHECK_UNARY(v2.is_integer());
        CHECK_UNARY(v3.is_integer());
        CHECK_EQ(v1.as_integer(), 123456);
        CHECK_EQ(v2.as_integer(), 2);
        CHECK_EQ(v3.as_integer(), 0xDEADBEEF);
    }
    {
        const toml::value v1 = "3.1415"_toml;
        const toml::value v2 = "6.02e+23"_toml;

        CHECK_UNARY(v1.is_floating());
        CHECK_UNARY(v2.is_floating());
        CHECK_EQ(v1.as_floating(), doctest::Approx(3.1415 ).epsilon(0.00001));
        CHECK_EQ(v2.as_floating(), doctest::Approx(6.02e23).epsilon(0.0001 ));
    }
    {
        const toml::value v1 = R"("foo")"_toml;
        const toml::value v2 = R"('foo')"_toml;
        const toml::value v3 = R"("""foo""")"_toml;
        const toml::value v4 = R"('''foo''')"_toml;

        CHECK_UNARY(v1.is_string());
        CHECK_UNARY(v2.is_string());
        CHECK_UNARY(v3.is_string());
        CHECK_UNARY(v4.is_string());
        CHECK_EQ(v1.as_string(), "foo");
        CHECK_EQ(v2.as_string(), "foo");
        CHECK_EQ(v3.as_string(), "foo");
        CHECK_EQ(v4.as_string(), "foo");
    }
    {
        {
            const toml::value v1 = R"([1,2,3])"_toml;
            CHECK_UNARY(v1.is_array());
            CHECK_EQ(v1.size(), 3);
            CHECK_EQ(v1.at(0).as_integer(), 1);
            CHECK_EQ(v1.at(1).as_integer(), 2);
            CHECK_EQ(v1.at(2).as_integer(), 3);
        }
        {
            const toml::value v2 = R"([1,])"_toml;
            CHECK_UNARY(v2.is_array());
            CHECK_EQ(v2.size(), 1);
            CHECK_EQ(v2.at(0).as_integer(),  1);
        }
        {
            const toml::value v3 = R"([[1,]])"_toml;
            CHECK_UNARY(v3.is_array());
            CHECK_EQ(v3.size(), 1);
            CHECK_UNARY(v3.at(0).is_array());
            CHECK_EQ(v3.at(0).at(0).as_integer(), 1);
        }
        {
            const toml::value v4 = R"([[1],])"_toml;
            CHECK_UNARY(v4.is_array());
            CHECK_EQ(v4.size(), 1);
            CHECK_UNARY(v4.at(0).is_array());
            CHECK_EQ(v4.at(0).at(0).as_integer(), 1);
        }
    }
    {
        const toml::value v1 = R"({a = 42})"_toml;
        CHECK_UNARY(v1.is_table());
        CHECK_EQ(v1.size(), 1);
        CHECK_UNARY(v1.at("a").is_integer());
        CHECK_EQ(v1.at("a").as_integer(), 42);
    }
    {
        const toml::value v1 = "1979-05-27"_toml;

        CHECK_UNARY(v1.is_local_date());
        CHECK_EQ(v1.as_local_date(), toml::local_date(1979, toml::month_t::May, 27));
    }
    {
        const toml::value v1 = "12:00:00"_toml;

        CHECK_UNARY(v1.is_local_time());
        CHECK_EQ(v1.as_local_time(), toml::local_time(12, 0, 0));
    }
    {
        const toml::value v1 = "1979-05-27T07:32:00"_toml;
        CHECK_UNARY(v1.is_local_datetime());
        CHECK_EQ(v1.as_local_datetime(), toml::local_datetime(
            toml::local_date(1979, toml::month_t::May, 27),
            toml::local_time(7, 32, 0)));
    }
    {
        const toml::value v1 = "1979-05-27T07:32:00Z"_toml;
        CHECK_UNARY(v1.is_offset_datetime());
        CHECK_EQ(v1.as_offset_datetime(), toml::offset_datetime(
            toml::local_date(1979, toml::month_t::May, 27),
            toml::local_time(7, 32, 0),
            toml::time_offset(0, 0)));
    }

}

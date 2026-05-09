#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/value.hpp>
#include <toml11/find.hpp>

#include <array>
#include <deque>
#include <list>
#include <map>
#include <tuple>
#include <unordered_map>

#if defined(TOML11_HAS_STRING_VIEW)
#include <string_view>
#endif

namespace toml
{
namespace detail
{
std::tm localtime_s(const std::time_t* src);
std::tm gmtime_s(const std::time_t* src);
} // detail
} // toml

TEST_CASE("testing toml::find with toml type")
{
    using value_type = toml::value;
    using boolean_type         = typename value_type::boolean_type        ;
    using integer_type         = typename value_type::integer_type        ;
    using floating_type        = typename value_type::floating_type       ;
    using string_type          = typename value_type::string_type         ;
    using local_time_type      = typename value_type::local_time_type     ;
    using local_date_type      = typename value_type::local_date_type     ;
    using local_datetime_type  = typename value_type::local_datetime_type ;
    using offset_datetime_type = typename value_type::offset_datetime_type;
    using array_type           = typename value_type::array_type          ;
    using table_type           = typename value_type::table_type          ;

    {
        value_type v(toml::table{{"a", true}});
        CHECK_EQ(true, toml::find<boolean_type>(v, "a"));

        toml::find<boolean_type>(v, "a") = false;
        CHECK_EQ(false, toml::find<boolean_type>(v, "a"));

        boolean_type x = toml::find<boolean_type>(std::move(v), "a");
        CHECK_EQ(false, x);
    }
    {
        value_type v(toml::table{{"a", 42}});
        CHECK_EQ(integer_type(42), toml::find<integer_type>(v, "a"));

        toml::find<integer_type>(v, "a") = 54;
        CHECK_EQ(integer_type(54), toml::find<integer_type>(v, "a"));

        integer_type x = toml::find<integer_type>(std::move(v), "a");
        CHECK_EQ(integer_type(54), x);
    }
    {
        value_type v(toml::table{{"a", 3.14}});
        CHECK_EQ(floating_type(3.14), toml::find<floating_type>(v, "a"));

        toml::find<floating_type>(v, "a") = 2.71;
        CHECK_EQ(floating_type(2.71), toml::find<floating_type>(v, "a"));

        floating_type x = toml::find<floating_type>(std::move(v), "a");
        CHECK_EQ(floating_type(2.71), x);
    }
    {
        value_type v(toml::table{{"a", "foo"}});
        CHECK_EQ("foo", toml::find<string_type>(v, "a"));

        toml::find<string_type>(v, "a") += "bar";
        CHECK_EQ("foobar", toml::find<string_type>(v, "a"));

        string_type x = toml::find<string_type>(std::move(v), "a");
        CHECK_EQ("foobar", x);
    }
    {
        local_date_type d(2018, toml::month_t::Apr, 22);
        value_type v(toml::table{{"a", d}});
        CHECK_EQ(d, toml::find<local_date_type>(v, "a"));

        toml::find<local_date_type>(v, "a").year = 2017;
        d.year = 2017;
        CHECK_EQ(d, toml::find<local_date_type>(v, "a"));

        local_date_type x = toml::find<local_date_type>(std::move(v), "a");
        CHECK_EQ(d, x);
    }
    {
        local_time_type t(12, 30, 45);
        value_type v(toml::table{{"a", t}});
        CHECK_EQ(t, toml::find<local_time_type>(v, "a"));

        toml::find<local_time_type>(v, "a").hour = 9;
        t.hour = 9;
        CHECK_EQ(t, toml::find<local_time_type>(v, "a"));

        local_time_type x = toml::find<local_time_type>(std::move(v), "a");
        CHECK_EQ(t, x);
    }
    {
        local_datetime_type dt(toml::local_date(2018, toml::month_t::Apr, 22),
                               toml::local_time(12, 30, 45));
        value_type v(toml::table{{"a", dt}});
        CHECK_EQ(dt, toml::find<local_datetime_type>(v, "a"));

        toml::find<local_datetime_type>(v, "a").date.year = 2017;
        dt.date.year = 2017;
        CHECK_EQ(dt, toml::find<local_datetime_type>(v, "a"));

        toml::local_datetime x = toml::find<local_datetime_type>(std::move(v), "a");
        CHECK_EQ(dt, x);
    }
    {
        offset_datetime_type dt(toml::local_datetime(
                    toml::local_date(2018, toml::month_t::Apr, 22),
                    toml::local_time(12, 30, 45)), toml::time_offset(9, 0));
        value_type v(toml::table{{"a", dt}});
        CHECK_EQ(dt, toml::find<offset_datetime_type>(v, "a"));

        toml::find<toml::offset_datetime>(v, "a").date.year = 2017;
        dt.date.year = 2017;
        CHECK_EQ(dt, toml::find<offset_datetime_type>(v, "a"));

        offset_datetime_type x = toml::find<offset_datetime_type>(std::move(v), "a");
        CHECK_EQ(dt, x);
    }
    {
        array_type vec;
        vec.push_back(value_type(42));
        vec.push_back(value_type(54));
        value_type v(toml::table{{"a", vec}});
        CHECK_EQ(vec, toml::find<array_type>(v, "a"));

        toml::find<array_type>(v, "a").push_back(value_type(123));
        vec.push_back(value_type(123));
        CHECK_EQ(vec, toml::find<array_type>(v, "a"));

        array_type x = toml::find<array_type>(std::move(v), "a");
        CHECK_EQ(vec, x);
    }
    {
        table_type tab;
        tab["key1"] = value_type(42);
        tab["key2"] = value_type(3.14);
        value_type v(toml::table{{"a", tab}});
        CHECK_EQ(tab, toml::find<table_type>(v, "a"));

        toml::find<table_type>(v, "a")["key3"] = value_type(123);
        tab["key3"] = value_type(123);
        CHECK_EQ(tab, toml::find<table_type>(v, "a"));

        table_type x = toml::find<table_type>(std::move(v), "a");
        CHECK_EQ(tab, x);
    }
    {
        value_type v1(toml::table{{"a", 42}});
        CHECK_EQ(toml::value(42), toml::find(v1, "a"));

        value_type v2(54);
        toml::find(v1, "a") = v2;
        CHECK_EQ(v2, toml::find(v1, "a"));

        value_type x = toml::find(std::move(v1), "a");
        CHECK_EQ(v2, x);
    }
}

TEST_CASE("testing find throws")
{
    using value_type = toml::value;
    using boolean_type         = typename value_type::boolean_type        ;
    using integer_type         = typename value_type::integer_type        ;

    // -----------------------------------------------------------------------
    // const-reference version
    {
        // value is not a table
        const toml::value v(true);
        CHECK_THROWS_AS(toml::find<boolean_type>(v, "key"), toml::type_error);
    }
    {
        // the value corresponding to the key is not the expected type
        const toml::value v = toml::table{{"key", 42}};
        CHECK_THROWS_AS(toml::find<boolean_type>(v, "key"), toml::type_error);
    }
    {
        // the value corresponding to the key is not found
        const toml::value v = toml::table{{"key", 42}};
        CHECK_THROWS_AS(toml::find<integer_type>(v, "different_key"),
                          std::out_of_range);
    }
    {
        // the positive control.
        const toml::value v = toml::table{{"key", 42}};
        CHECK_EQ(42, toml::find<int>(v, "key"));
    }

    // -----------------------------------------------------------------------
    // reference version
    {
        // value is not a table
        toml::value v(true);
        CHECK_THROWS_AS(toml::find<boolean_type>(v, "key"), toml::type_error);
    }
    {
        // the value corresponding to the key is not the expected type
        toml::value v = toml::table{{"key", 42}};
        CHECK_THROWS_AS(toml::find<boolean_type>(v, "key"), toml::type_error);
    }
    {
        // the value corresponding to the key is not found
        toml::value v = toml::table{{"key", 42}};
        CHECK_THROWS_AS(toml::find<integer_type>(v, "different_key"),
                          std::out_of_range);
    }
    {
        // the positive control.
        toml::value v = toml::table{{"key", 42}};
        CHECK_EQ(42, toml::find<int>(v, "key"));
    }

    // -----------------------------------------------------------------------
    // move version

    {
        // value is not a table
        toml::value v(true);
        CHECK_THROWS_AS(toml::find<boolean_type>(std::move(v), "key"), toml::type_error);
    }
    {
        // the value corresponding to the key is not the expected type
        toml::value v = toml::table{{"key", 42}};
        CHECK_THROWS_AS(toml::find<boolean_type>(std::move(v), "key"), toml::type_error);
    }
    {
        // the value corresponding to the key is not found
        toml::value v = toml::table{{"key", 42}};
        CHECK_THROWS_AS(toml::find<integer_type>(std::move(v), "different_key"),
                          std::out_of_range);
    }
    {
        // the positive control.
        toml::value v = toml::table{{"key", 42}};
        CHECK_EQ(42, toml::find<int>(std::move(v), "key"));
    }
}

TEST_CASE("testing toml::find(v, idx) throws")
{
    using value_type = toml::value;
    using boolean_type         = typename value_type::boolean_type        ;
    using integer_type         = typename value_type::integer_type        ;

    // -----------------------------------------------------------------------
    // const-reference version
    {
        // value is not an array
        const toml::value v(true);
        CHECK_THROWS_AS(toml::find<boolean_type>(v, 0), toml::type_error);
    }
    {
        // the value corresponding to the key is not the expected type
        const toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_THROWS_AS(toml::find<boolean_type>(v, 0), toml::type_error);
    }
    {
        // the value corresponding to the key is not found
        const toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_THROWS_AS(toml::find<integer_type>(v, 6), std::out_of_range);
    }
    {
        // the positive control.
        const toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_EQ(3, toml::find<int>(v, 2));
    }

    // -----------------------------------------------------------------------
    // non-const reference version
    {
        // value is not an array
        toml::value v(true);
        CHECK_THROWS_AS(toml::find<boolean_type>(v, 0), toml::type_error);
    }
    {
        // the value corresponding to the key is not the expected type
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_THROWS_AS(toml::find<boolean_type>(v, 0), toml::type_error);
    }
    {
        // the value corresponding to the key is not found
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_THROWS_AS(toml::find<integer_type>(v, 6), std::out_of_range);
    }
    {
        // the positive control.
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_EQ(3, toml::find<int>(v, 2));
    }

    // -----------------------------------------------------------------------
    // move version
    {
        // value is not an array
        toml::value v(true);
        CHECK_THROWS_AS(toml::find<boolean_type>(std::move(v), 0), toml::type_error);
    }
    {
        // the value corresponding to the key is not the expected type
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_THROWS_AS(toml::find<boolean_type>(std::move(v), 0), toml::type_error);
    }
    {
        // the value corresponding to the key is not found
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_THROWS_AS(toml::find<integer_type>(std::move(v), 6), std::out_of_range);
    }
    {
        // the positive control.
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_EQ(3, toml::find<int>(std::move(v), 2));
    }
}

TEST_CASE("testing toml::find with recursive table/array")
{
    using value_type = toml::value;
    using integer_type         = typename value_type::integer_type        ;

    // recursively search tables
    {
        toml::value v = toml::table{
            {"a", toml::table{
                {"b", toml::table{
                    {"c", toml::table{
                        {"d", 42}
                    }}
                }}
            }}
        };
        CHECK_EQ(42, toml::find<int>(v, "a", "b", "c", "d"));

        // reference that can be used to modify the content
        auto& num = toml::find<integer_type>(v, "a", "b", "c", "d");
        num = 54;
        CHECK_EQ(54, toml::find<int>(v, "a", "b", "c", "d"));

        const std::string a("a"), b("b"), c("c"), d("d");
        auto& num2 = toml::find<integer_type>(v, a, b, c, d);
        num2 = 42;
        CHECK_EQ(42, toml::find<int>(v, a, b, c, d));

        auto num3 = toml::find<integer_type>(v, a, "b", c, "d");
        CHECK_EQ(42, num3);

        auto num4 = toml::find<integer_type>(std::move(v), a, b, c, d);
        CHECK_EQ(42, num4);
    }
    // recursively search arrays
    {
        toml::value v = toml::array{
            toml::array{"array", "of", "string"},
            toml::array{toml::array{1, 2, 3}, toml::array{3.14, 2.71}}
        };
        CHECK_EQ("array" , toml::find<std::string>(v, 0, 0));
        CHECK_EQ("of"    , toml::find<std::string>(v, 0, 1));
        CHECK_EQ("string", toml::find<std::string>(v, 0, 2));

        CHECK_EQ(1, toml::find<int>(v, 1, 0, 0));
        CHECK_EQ(2, toml::find<int>(v, 1, 0, 1));
        CHECK_EQ(3, toml::find<int>(v, 1, 0, 2));

        CHECK_EQ(3.14, toml::find<double>(v, 1, 1, 0));
        CHECK_EQ(2.71, toml::find<double>(v, 1, 1, 1));

        // reference that can be used to modify the content
        auto& num = toml::find<integer_type>(v, 1, 0, 2);
        num = 42;
        CHECK_EQ( 1, toml::find<int>(v, 1, 0, 0));
        CHECK_EQ( 2, toml::find<int>(v, 1, 0, 1));
        CHECK_EQ(42, toml::find<int>(v, 1, 0, 2));

        // move value
        auto num2 = toml::find<integer_type>(std::move(v), 1, 0, 2);
        CHECK_EQ(42, num2);
    }
    // recursively search mixtures
    {
        toml::value v = toml::table{{"array", toml::array{
                toml::array{1,   2,   3},
                toml::array{
                    toml::table{{"foo", "bar"}, {"baz", "qux"}},
                    toml::table{{"pi",   3.14}, {"e",    2.71}}
                }}
            }};

        CHECK_EQ(1, toml::find<int>(v, "array", 0, 0));
        CHECK_EQ(2, toml::find<int>(v, "array", 0, 1));
        CHECK_EQ(3, toml::find<int>(v, "array", 0, 2));

        CHECK_EQ("bar", toml::find<std::string>(v, "array", 1, 0, "foo"));
        CHECK_EQ("qux", toml::find<std::string>(v, "array", 1, 0, "baz"));

        CHECK_EQ(3.14, toml::find<double>(v, "array", 1, 1, "pi"));
        CHECK_EQ(2.71, toml::find<double>(v, "array", 1, 1, "e"));

        const std::string ar("array");
        const auto ar_c = "array";

        const std::string pi("pi");
        const auto pi_c = "pi";

        CHECK_EQ(3.14, toml::find<double>(v, ar, 1, 1, "pi"));
        CHECK_EQ(3.14, toml::find<double>(v, ar, 1, 1, pi));
        CHECK_EQ(3.14, toml::find<double>(v, ar, 1, 1, pi_c));

        CHECK_EQ(3.14, toml::find<double>(v, ar_c, 1, 1, "pi"));
        CHECK_EQ(3.14, toml::find<double>(v, ar_c, 1, 1, pi));
        CHECK_EQ(3.14, toml::find<double>(v, ar_c, 1, 1, pi_c));

        CHECK_EQ(3.14, toml::find<double>(v, "array", 1, 1, pi));
        CHECK_EQ(3.14, toml::find<double>(v, "array", 1, 1, pi_c));
    }
}

TEST_CASE("testing toml::find with toml types")
{
    using value_type = toml::value;
    using boolean_type         = typename value_type::boolean_type        ;
    using integer_type         = typename value_type::integer_type        ;
    using floating_type        = typename value_type::floating_type       ;
    using string_type          = typename value_type::string_type         ;
    using local_time_type      = typename value_type::local_time_type     ;
    using local_date_type      = typename value_type::local_date_type     ;
    using local_datetime_type  = typename value_type::local_datetime_type ;
    using offset_datetime_type = typename value_type::offset_datetime_type;
    using array_type           = typename value_type::array_type          ;
    using table_type           = typename value_type::table_type          ;

    {
        value_type v = toml::table{{"key", true}};
        CHECK(true == toml::find<boolean_type>(v, "key"));

        toml::find<boolean_type>(v, "key") = false;
        CHECK(false == toml::find<boolean_type>(v, "key"));

        const auto moved = toml::find<boolean_type>(std::move(v), "key");
        CHECK(false == moved);
    }
    {
        value_type v = toml::table{{"key", 42}};
        CHECK(integer_type(42) == toml::find<integer_type>(v, "key"));

        toml::find<integer_type>(v, "key") = 54;
        CHECK(integer_type(54) == toml::find<integer_type>(v, "key"));

        const auto moved = toml::find<integer_type>(std::move(v), "key");
        CHECK(integer_type(54) == moved);
    }
    {
        value_type v = toml::table{{"key", 3.14}};
        CHECK(floating_type(3.14) == toml::find<floating_type>(v, "key"));

        toml::find<floating_type>(v, "key") = 2.71;
        CHECK(floating_type(2.71) == toml::find<floating_type>(v, "key"));

        const auto moved = toml::find<floating_type>(std::move(v), "key");
        CHECK(floating_type(2.71) == moved);
    }
    {
        value_type v = toml::table{{"key", "foo"}};
        CHECK("foo" == toml::find<string_type>(v, "key"));

        toml::find<string_type>(v, "key") += "bar";
        CHECK("foobar" == toml::find<string_type>(v, "key"));

        const auto moved = toml::find<string_type>(std::move(v), "key");
        CHECK("foobar" == moved);
    }
    {
        local_date_type d(2018, toml::month_t::Apr, 22);
        value_type v = toml::table{{"key", d}};
        CHECK(d == toml::find<local_date_type>(v, "key"));

        toml::find<local_date_type>(v, "key").year = 2017;
        d.year = 2017;
        CHECK(d == toml::find<local_date_type>(v, "key"));

        const auto moved = toml::find<local_date_type>(std::move(v), "key");
        CHECK(d == moved);
    }
    {
        local_time_type t(12, 30, 45);
        value_type v = toml::table{{"key", t}};
        CHECK(t == toml::find<local_time_type>(v, "key"));

        toml::find<local_time_type>(v, "key").hour = 9;
        t.hour = 9;
        CHECK(t == toml::find<local_time_type>(v, "key"));

        const auto moved = toml::find<local_time_type>(std::move(v), "key");
        CHECK(t == moved);
    }
    {
        local_datetime_type dt(toml::local_date(2018, toml::month_t::Apr, 22),
                                local_time_type(12, 30, 45));
        value_type v = toml::table{{"key", dt}};
        CHECK(dt == toml::find<local_datetime_type>(v, "key"));

        toml::find<local_datetime_type>(v, "key").date.year = 2017;
        dt.date.year = 2017;
        CHECK(dt == toml::find<local_datetime_type>(v, "key"));

        const auto moved = toml::find<local_datetime_type>(std::move(v), "key");
        CHECK(dt == moved);
    }
    {
        offset_datetime_type dt(local_datetime_type(
                    toml::local_date(2018, toml::month_t::Apr, 22),
                    local_time_type(12, 30, 45)), toml::time_offset(9, 0));
        value_type v = toml::table{{"key", dt}};
        CHECK(dt == toml::find<offset_datetime_type>(v, "key"));

        toml::find<offset_datetime_type>(v, "key").date.year = 2017;
        dt.date.year = 2017;
        CHECK(dt == toml::find<offset_datetime_type>(v, "key"));

        const auto moved = toml::find<offset_datetime_type>(std::move(v), "key");
        CHECK(dt == moved);
    }
    {
        array_type vec;
        vec.push_back(value_type(42));
        vec.push_back(value_type(54));
        value_type v = toml::table{{"key", vec}};

        const bool result1 = (vec == toml::find<array_type>(v, "key"));
        CHECK(result1);

        toml::find<array_type>(v, "key").push_back(value_type(123));
        vec.push_back(value_type(123));

        const bool result2 = (vec == toml::find<array_type>(v, "key"));
        CHECK(result2);

        const auto moved = toml::find<array_type>(std::move(v), "key");
        const bool result3 = (vec == moved);
        CHECK(result3);
    }
    {
        table_type tab;
        tab["key1"] = value_type(42);
        tab["key2"] = value_type(3.14);
        value_type v = toml::table{{"key", tab}};
        const bool result1 = (tab == toml::find<table_type>(v, "key"));
        CHECK(result1);

        toml::find<table_type>(v, "key")["key3"] = value_type(123);
        tab["key3"] = value_type(123);
        const bool result2 = (tab == toml::find<table_type>(v, "key"));
        CHECK(result2);

        const auto moved = toml::find<table_type>(std::move(v), "key");
        const bool result3 = (tab == moved);
        CHECK(result3);
    }
    {
        value_type v1(42);
        value_type v = toml::table{{"key", v1}};
        CHECK(v1 == toml::find(v, "key"));

        value_type v2(54);
        toml::find(v, "key") = v2;
        CHECK(v2 == toml::find(v, "key"));

        const auto moved = toml::find(std::move(v), "key");
        CHECK(v2 == moved);
    }
}

TEST_CASE("testing toml::find integer conversion")
{
    using value_type = toml::value;

    {
        value_type v = toml::table{{"key", 42}};
        CHECK_EQ(int(42)          , toml::find<int          >(v, "key"));
        CHECK_EQ(short(42)        , toml::find<short        >(v, "key"));
        CHECK_EQ(char(42)         , toml::find<char         >(v, "key"));
        CHECK_EQ(unsigned(42)     , toml::find<unsigned     >(v, "key"));
        CHECK_EQ(long(42)         , toml::find<long         >(v, "key"));
        CHECK_EQ(std::int64_t(42) , toml::find<std::int64_t >(v, "key"));
        CHECK_EQ(std::uint64_t(42), toml::find<std::uint64_t>(v, "key"));
        CHECK_EQ(std::int16_t(42) , toml::find<std::int16_t >(v, "key"));
        CHECK_EQ(std::uint16_t(42), toml::find<std::uint16_t>(v, "key"));
        CHECK_EQ(std::uint16_t(42), toml::find<std::uint16_t>(std::move(v), "key"));
    }
}

TEST_CASE("testing toml::find floating conversion")
{
    using value_type = toml::value;

    {
        value_type v = toml::table{{"key", 3.14}};
        const double ref(3.14);
        CHECK_EQ(static_cast<float      >(ref), toml::find<float      >(v, "key"));
        CHECK_EQ(                         ref , toml::find<double     >(v, "key"));
        CHECK_EQ(static_cast<long double>(ref), toml::find<long double>(v, "key"));
        CHECK_EQ(static_cast<float      >(ref), toml::find<float      >(std::move(v), "key"));
    }
}

TEST_CASE("testing toml::find string conversion")
{
    using value_type = toml::value;

    {
        value_type v = toml::table{{"key", "foo"}};
        CHECK_EQ("foo", toml::find<std::string>(v, "key"));
        toml::find<std::string>(v, "key") += "bar";
        CHECK_EQ("foobar", toml::find<std::string>(v, "key"));
    }
}

#if defined(TOML11_HAS_CHAR8_T)
TEST_CASE("testing toml::find<string-like>")
{
    using value_type = toml::value;
    {
        value_type v = toml::table{{"key", "foo"}};
        CHECK_EQ(u8"foo", toml::find<std::u8string>(v, "key"));
    }
}
#endif

#if TOML11_CPLUSPLUS_STANDARD_VERSION >= 201703L
TEST_CASE("testing toml::get<string_view>")
{
    using value_type = toml::value;
    {
        value_type v("foo");
        CHECK_EQ("foo", toml::get<std::string_view>(v));
    }
}
#endif

TEST_CASE("testing toml::find array conversion")
{
    using value_type = toml::value;

    value_type v = toml::table{{"key", toml::array{42, 54, 69, 72}}};

    const std::vector<int>         vec = toml::find<std::vector<int>>(v, "key");
    const std::list<short>         lst = toml::find<std::list<short>>(v, "key");
    const std::deque<std::int64_t> deq = toml::find<std::deque<std::int64_t>>(v, "key");

    CHECK_EQ(42, vec.at(0));
    CHECK_EQ(54, vec.at(1));
    CHECK_EQ(69, vec.at(2));
    CHECK_EQ(72, vec.at(3));

    std::list<short>::const_iterator iter = lst.begin();
    CHECK_EQ(static_cast<short>(42), *(iter++));
    CHECK_EQ(static_cast<short>(54), *(iter++));
    CHECK_EQ(static_cast<short>(69), *(iter++));
    CHECK_EQ(static_cast<short>(72), *(iter++));

    CHECK_EQ(static_cast<std::int64_t>(42), deq.at(0));
    CHECK_EQ(static_cast<std::int64_t>(54), deq.at(1));
    CHECK_EQ(static_cast<std::int64_t>(69), deq.at(2));
    CHECK_EQ(static_cast<std::int64_t>(72), deq.at(3));

    std::array<int, 4> ary = toml::find<std::array<int, 4>>(v, "key");
    CHECK_EQ(42, ary.at(0));
    CHECK_EQ(54, ary.at(1));
    CHECK_EQ(69, ary.at(2));
    CHECK_EQ(72, ary.at(3));

    std::tuple<int, short, unsigned, long> tpl =
        toml::find<std::tuple<int, short, unsigned, long>>(v, "key");
    CHECK_EQ(                      42 , std::get<0>(tpl));
    CHECK_EQ(static_cast<short   >(54), std::get<1>(tpl));
    CHECK_EQ(static_cast<unsigned>(69), std::get<2>(tpl));
    CHECK_EQ(static_cast<long    >(72), std::get<3>(tpl));

    value_type p = toml::table{{"key", toml::array{3.14, 2.71}}};
    std::pair<double, double> pr = toml::find<std::pair<double, double> >(p, "key");
    CHECK_EQ(3.14, pr.first);
    CHECK_EQ(2.71, pr.second);
}

TEST_CASE("testing toml::find array move conversion")
{
    using value_type = toml::value;

    value_type v1 = toml::table{{"key", toml::array{42, 54, 69, 72}}};
    value_type v2 = toml::table{{"key", toml::array{42, 54, 69, 72}}};
    value_type v3 = toml::table{{"key", toml::array{42, 54, 69, 72}}};
    value_type v4 = toml::table{{"key", toml::array{42, 54, 69, 72}}};
    value_type v5 = toml::table{{"key", toml::array{42, 54, 69, 72}}};

    const std::vector<int>         vec = toml::find<std::vector<int>>(std::move(v1), "key");
    const std::list<short>         lst = toml::find<std::list<short>>(std::move(v2), "key");
    const std::deque<std::int64_t> deq = toml::find<std::deque<std::int64_t>>(std::move(v3), "key");

    CHECK_EQ(42, vec.at(0));
    CHECK_EQ(54, vec.at(1));
    CHECK_EQ(69, vec.at(2));
    CHECK_EQ(72, vec.at(3));

    std::list<short>::const_iterator iter = lst.begin();
    CHECK_EQ(static_cast<short>(42), *(iter++));
    CHECK_EQ(static_cast<short>(54), *(iter++));
    CHECK_EQ(static_cast<short>(69), *(iter++));
    CHECK_EQ(static_cast<short>(72), *(iter++));

    CHECK_EQ(static_cast<std::int64_t>(42), deq.at(0));
    CHECK_EQ(static_cast<std::int64_t>(54), deq.at(1));
    CHECK_EQ(static_cast<std::int64_t>(69), deq.at(2));
    CHECK_EQ(static_cast<std::int64_t>(72), deq.at(3));

    std::array<int, 4> ary = toml::find<std::array<int, 4>>(std::move(v4), "key");
    CHECK_EQ(42, ary.at(0));
    CHECK_EQ(54, ary.at(1));
    CHECK_EQ(69, ary.at(2));
    CHECK_EQ(72, ary.at(3));

    std::tuple<int, short, unsigned, long> tpl =
        toml::find<std::tuple<int, short, unsigned, long>>(std::move(v5), "key");
    CHECK_EQ(                      42 , std::get<0>(tpl));
    CHECK_EQ(static_cast<short   >(54), std::get<1>(tpl));
    CHECK_EQ(static_cast<unsigned>(69), std::get<2>(tpl));
    CHECK_EQ(static_cast<long    >(72), std::get<3>(tpl));

    value_type p = toml::table{{"key", toml::array{3.14, 2.71}}};
    std::pair<double, double> pr = toml::find<std::pair<double, double> >(std::move(p), "key");
    CHECK_EQ(3.14, pr.first);
    CHECK_EQ(2.71, pr.second);
}

TEST_CASE("testing toml::find array of array conversion")
{
    using value_type = toml::value;

    value_type v1 = toml::array{42, 54, 69, 72};
    value_type v2 = toml::array{"foo", "bar", "baz"};
    value_type v = toml::table{{"key", toml::array{v1, v2}}};

    std::pair<std::vector<int>, std::vector<std::string>> p =
        toml::find<std::pair<std::vector<int>, std::vector<std::string>>>(v, "key");

    CHECK_EQ(p.first.at(0), 42);
    CHECK_EQ(p.first.at(1), 54);
    CHECK_EQ(p.first.at(2), 69);
    CHECK_EQ(p.first.at(3), 72);

    CHECK_EQ(p.second.at(0), "foo");
    CHECK_EQ(p.second.at(1), "bar");
    CHECK_EQ(p.second.at(2), "baz");

    std::tuple<std::vector<int>, std::vector<std::string>> t =
        toml::find<std::tuple<std::vector<int>, std::vector<std::string>>>(v, "key");

    CHECK_EQ(std::get<0>(t).at(0), 42);
    CHECK_EQ(std::get<0>(t).at(1), 54);
    CHECK_EQ(std::get<0>(t).at(2), 69);
    CHECK_EQ(std::get<0>(t).at(3), 72);

    CHECK_EQ(std::get<1>(t).at(0), "foo");
    CHECK_EQ(std::get<1>(t).at(1), "bar");
    CHECK_EQ(std::get<1>(t).at(2), "baz");
}

TEST_CASE("testing toml::find array of array move conversion")
{
    using value_type = toml::value;

    value_type a1 = toml::array{42, 54, 69, 72};
    value_type a2 = toml::array{"foo", "bar", "baz"};
    value_type v1 = toml::table{{"key", toml::array{a1, a2}}};
    value_type v2 = toml::table{{"key", toml::array{a1, a2}}};

    std::pair<std::vector<int>, std::vector<std::string>> p =
        toml::find<std::pair<std::vector<int>, std::vector<std::string>>>(std::move(v1), "key");

    CHECK_EQ(p.first.at(0), 42);
    CHECK_EQ(p.first.at(1), 54);
    CHECK_EQ(p.first.at(2), 69);
    CHECK_EQ(p.first.at(3), 72);

    CHECK_EQ(p.second.at(0), "foo");
    CHECK_EQ(p.second.at(1), "bar");
    CHECK_EQ(p.second.at(2), "baz");

    std::tuple<std::vector<int>, std::vector<std::string>> t =
        toml::find<std::tuple<std::vector<int>, std::vector<std::string>>>(std::move(v2), "key");

    CHECK_EQ(std::get<0>(t).at(0), 42);
    CHECK_EQ(std::get<0>(t).at(1), 54);
    CHECK_EQ(std::get<0>(t).at(2), 69);
    CHECK_EQ(std::get<0>(t).at(3), 72);

    CHECK_EQ(std::get<1>(t).at(0), "foo");
    CHECK_EQ(std::get<1>(t).at(1), "bar");
    CHECK_EQ(std::get<1>(t).at(2), "baz");
}

TEST_CASE("testing toml::find table conversion")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"key", toml::table{
                {"key1", 1}, {"key2", 2}, {"key3", 3}, {"key4", 4}
            }}};
        const auto v = toml::find<std::map<std::string, int>>(v1, "key");
        CHECK_EQ(v.at("key1"), 1);
        CHECK_EQ(v.at("key2"), 2);
        CHECK_EQ(v.at("key3"), 3);
        CHECK_EQ(v.at("key4"), 4);
    }
    {
        value_type v1 = toml::table{{"key", toml::table{
                {"key1", 1}, {"key2", 2}, {"key3", 3}, {"key4", 4}
            }}};
        const auto v = toml::find<std::map<std::string, int>>(std::move(v1), "key");
        CHECK_EQ(v.at("key1"), 1);
        CHECK_EQ(v.at("key2"), 2);
        CHECK_EQ(v.at("key3"), 3);
        CHECK_EQ(v.at("key4"), 4);
    }
}

TEST_CASE("testing toml::find local_date")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"key", toml::local_date{2018, toml::month_t::Apr, 1}}};
        const auto date = std::chrono::system_clock::to_time_t(
                toml::find<std::chrono::system_clock::time_point>(v1, "key"));

        std::tm t;
        t.tm_year = 2018 - 1900;
        t.tm_mon  = 4    - 1;
        t.tm_mday = 1;
        t.tm_hour = 0;
        t.tm_min  = 0;
        t.tm_sec  = 0;
        t.tm_isdst = -1;
        const auto c = std::mktime(&t);
        CHECK_EQ(c, date);
    }
    {
        value_type v1 = toml::table{{"key", toml::local_date{2018, toml::month_t::Apr, 1}}};
        const auto date = std::chrono::system_clock::to_time_t(
                toml::find<std::chrono::system_clock::time_point>(std::move(v1), "key"));

        std::tm t;
        t.tm_year = 2018 - 1900;
        t.tm_mon  = 4    - 1;
        t.tm_mday = 1;
        t.tm_hour = 0;
        t.tm_min  = 0;
        t.tm_sec  = 0;
        t.tm_isdst = -1;
        const auto c = std::mktime(&t);
        CHECK_EQ(c, date);
    }
}

TEST_CASE("testing toml::find local_time")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"key", toml::local_time{12, 30, 45}}};
        const auto time = toml::find<std::chrono::seconds>(v1, "key");
        CHECK_EQ(time, std::chrono::hours(12) +
                            std::chrono::minutes(30) + std::chrono::seconds(45));
    }
    {
        value_type v1 = toml::table{{"key", toml::local_time{12, 30, 45}}};
        const auto time = toml::find<std::chrono::seconds>(std::move(v1), "key");
        CHECK_EQ(time, std::chrono::hours(12) +
                            std::chrono::minutes(30) + std::chrono::seconds(45));
    }
}

TEST_CASE("testing toml::find local_datetime")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"key", toml::local_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 45})}};

        const auto date = std::chrono::system_clock::to_time_t(
                toml::find<std::chrono::system_clock::time_point>(v1, "key"));
        std::tm t;
        t.tm_year = 2018 - 1900;
        t.tm_mon  = 4    - 1;
        t.tm_mday = 1;
        t.tm_hour = 12;
        t.tm_min  = 30;
        t.tm_sec  = 45;
        t.tm_isdst = -1;
        const auto c = std::mktime(&t);
        CHECK_EQ(c,  date);
    }
    {
        value_type v1 = toml::table{{"key", toml::local_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 45})}};

        const auto date = std::chrono::system_clock::to_time_t(
                toml::find<std::chrono::system_clock::time_point>(std::move(v1), "key"));
        std::tm t;
        t.tm_year = 2018 - 1900;
        t.tm_mon  = 4    - 1;
        t.tm_mday = 1;
        t.tm_hour = 12;
        t.tm_min  = 30;
        t.tm_sec  = 45;
        t.tm_isdst = -1;
        const auto c = std::mktime(&t);
        CHECK_EQ(c,  date);
    }
}

TEST_CASE("testing toml::find offset_datetime")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"key", toml::offset_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 0},
                    toml::time_offset{9, 0})}};
        //    2018-04-01T12:30:00+09:00
        //,  2018-04-01T03:30:00Z

        const auto date = toml::find<std::chrono::system_clock::time_point>(v1, "key");
        const auto timet = std::chrono::system_clock::to_time_t(date);

        // get time_t as gmtime (2018-04-01T03:30:00Z)
        const auto tm = toml::detail::gmtime_s(std::addressof(timet));
        CHECK_EQ(tm.tm_year + 1900,  2018);
        CHECK_EQ(tm.tm_mon  + 1,        4);
        CHECK_EQ(tm.tm_mday,            1);
        CHECK_EQ(tm.tm_hour,            3);
        CHECK_EQ(tm.tm_min,            30);
        CHECK_EQ(tm.tm_sec,             0);
    }

    {
        value_type v1 = toml::table{{"key", toml::offset_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 0},
                    toml::time_offset{-8, 0})}};
        //    2018-04-01T12:30:00-08:00
        //,  2018-04-01T20:30:00Z

        const auto date = toml::find<std::chrono::system_clock::time_point>(v1, "key");
        const auto timet = std::chrono::system_clock::to_time_t(date);

        // get time_t as gmtime (2018-04-01T03:30:00Z)
        const auto tm = toml::detail::gmtime_s(std::addressof(timet));
        CHECK_EQ(tm.tm_year + 1900,  2018);
        CHECK_EQ(tm.tm_mon  + 1,        4);
        CHECK_EQ(tm.tm_mday,            1);
        CHECK_EQ(tm.tm_hour,           20);
        CHECK_EQ(tm.tm_min,            30);
        CHECK_EQ(tm.tm_sec,             0);
    }

    {
        value_type v1 = toml::table{{"key", toml::offset_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 0},
                    toml::time_offset{-8, 0})}};
        //    2018-04-01T12:30:00-08:00
        //,  2018-04-01T20:30:00Z

        const auto date = toml::find<std::chrono::system_clock::time_point>(std::move(v1), "key");
        const auto timet = std::chrono::system_clock::to_time_t(date);

        // get time_t as gmtime (2018-04-01T03:30:00Z)
        const auto tm = toml::detail::gmtime_s(std::addressof(timet));
        CHECK_EQ(tm.tm_year + 1900,  2018);
        CHECK_EQ(tm.tm_mon  + 1,        4);
        CHECK_EQ(tm.tm_mday,            1);
        CHECK_EQ(tm.tm_hour,           20);
        CHECK_EQ(tm.tm_min,            30);
        CHECK_EQ(tm.tm_sec,             0);
    }
}

#ifdef TOML11_HAS_OPTIONAL
TEST_CASE("testing toml::find<optional>")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"foo", "hoge"}};

        const auto foo = toml::find<std::optional<std::string>>(v1, "foo");
        CHECK_UNARY(foo.has_value());
        CHECK_EQ(foo.value(), "hoge");

        const auto bar = toml::find<std::optional<std::string>>(v1, "bar");
        CHECK_UNARY_FALSE(bar.has_value());

        CHECK_THROWS_AS(toml::find<std::optional<double>>(v1, "foo"), toml::type_error);
    }
    {
        value_type v1 = toml::table{{ "foo", toml::array{1,2,3,4,5} }};

        const auto foo = toml::find<std::optional<std::array<int, 5>>>(v1, "foo");
        CHECK_UNARY(foo.has_value());
        CHECK_EQ(foo.value().at(0), 1);
        CHECK_EQ(foo.value().at(1), 2);
        CHECK_EQ(foo.value().at(2), 3);
        CHECK_EQ(foo.value().at(3), 4);
        CHECK_EQ(foo.value().at(4), 5);

        const auto bar = toml::find<std::optional<std::array<int, 5>>>(v1, "bar");
        CHECK_UNARY_FALSE(bar.has_value());

        CHECK_THROWS_AS(toml::find<std::optional<double>>(v1, "foo"), toml::type_error);
    }
    {
        value_type v1 = toml::array{1,2,3,4,5};

        const auto foo = toml::find<std::optional<int>>(v1, 2);
        CHECK_UNARY(foo.has_value());
        CHECK_EQ(foo.value(), 3);

        const auto bar = toml::find<std::optional<int>>(v1, 5);
        CHECK_UNARY_FALSE(bar.has_value());

        CHECK_THROWS_AS(toml::find<std::optional<double>>(v1, 2), toml::type_error);
    }
    {
        value_type v1 = toml::table{
            { "foo", toml::table{
                {"bar", 3.14},
                {"baz", 2.71}
            }}
        };

        const auto foobar = toml::find<std::optional<double>>(v1, "foo", "bar");
        CHECK_UNARY(foobar.has_value());
        CHECK_EQ(foobar.value(), 3.14);

        const auto foobaz = toml::find<std::optional<double>>(v1, "foo", "baz");
        CHECK_UNARY(foobaz.has_value());
        CHECK_EQ(foobaz.value(), 2.71);

        const auto fooqux = toml::find<std::optional<double>>(v1, "foo", "qux");
        CHECK_UNARY_FALSE(fooqux.has_value());

        const auto barfoo = toml::find<std::optional<double>>(v1, "bar", "foo");
        CHECK_UNARY_FALSE(barfoo.has_value());

        CHECK_THROWS_AS(toml::find<std::optional<int>>(v1, "foo", "bar"), toml::type_error);
        CHECK_THROWS_AS(toml::find<std::optional<double>>(v1, "foo", 1),  toml::type_error);
        CHECK_THROWS_AS(toml::find<std::optional<double>>(v1, "foo", "bar", "baz"), toml::type_error);
    }
    {
        value_type v1 = toml::table{
            { "foo", toml::array{
                toml::table{
                    {"bar", 3.14}
                },
                toml::table{
                    {"bar", 2.71}
                }
            }}
        };

        const auto foobar = toml::find<std::optional<double>>(v1, "foo", 0, "bar");
        CHECK_UNARY(foobar.has_value());
        CHECK_EQ(foobar.value(), 3.14);

        const auto foobaz = toml::find<std::optional<double>>(v1, "foo", 1, "bar");
        CHECK_UNARY(foobaz.has_value());
        CHECK_EQ(foobaz.value(), 2.71);

        const auto barbar = toml::find<std::optional<double>>(v1, "bar", 0, "bar");
        CHECK_UNARY_FALSE(barbar.has_value());

        const auto foo2bar = toml::find<std::optional<double>>(v1, "foo", 2, "bar");
        CHECK_UNARY_FALSE(foo2bar.has_value());

        const auto foo0baz = toml::find<std::optional<double>>(v1, "foo", 0, "baz");
        CHECK_UNARY_FALSE(foo0baz.has_value());

        CHECK_THROWS_AS(toml::find<std::optional<int>>(v1, "foo", "bar", "baz"), toml::type_error);
    }
}
#endif

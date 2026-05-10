#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/value.hpp>
#include <toml11/get.hpp>

#include <array>
#include <deque>
#include <list>
#include <map>
#include <tuple>
#include <unordered_map>

#if TOML11_CPLUSPLUS_STANDARD_VERSION >= 201703L
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

TEST_CASE("testing toml::get with toml types")
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
        value_type v(true);
        CHECK_EQ(true, toml::get<boolean_type>(v));

        toml::get<boolean_type>(v) = false;
        CHECK_EQ(false, toml::get<boolean_type>(v));

        boolean_type x = toml::get<boolean_type>(std::move(v));
        CHECK_EQ(false, x);
    }
    {
        value_type v(42);
        CHECK_EQ(integer_type(42), toml::get<integer_type>(v));

        toml::get<integer_type>(v) = 54;
        CHECK_EQ(integer_type(54), toml::get<integer_type>(v));

        integer_type x = toml::get<integer_type>(std::move(v));
        CHECK_EQ(integer_type(54), x);
    }
    {
        value_type v(3.14);
        CHECK_EQ(floating_type(3.14), toml::get<floating_type>(v));

        toml::get<floating_type>(v) = 2.71;
        CHECK_EQ(floating_type(2.71), toml::get<floating_type>(v));

        floating_type x = toml::get<floating_type>(std::move(v));
        CHECK_EQ(floating_type(2.71), x);
    }
    {
        value_type v("foo");
        CHECK_EQ("foo", toml::get<string_type>(v));

        toml::get<string_type>(v) += "bar";
        CHECK_EQ("foobar", toml::get<string_type>(v));

        string_type x = toml::get<string_type>(std::move(v));
        CHECK_EQ("foobar", x);
    }
    {
        local_date_type d(2018, toml::month_t::Apr, 22);
        value_type v(d);
        CHECK_EQ(d, toml::get<local_date_type>(v));

        toml::get<local_date_type>(v).year = 2017;
        d.year = 2017;
        CHECK_EQ(d, toml::get<local_date_type>(v));

        local_date_type x = toml::get<local_date_type>(std::move(v));
        CHECK_EQ(d, x);
    }
    {
        local_time_type t(12, 30, 45);
        value_type v(t);
        CHECK_EQ(t, toml::get<local_time_type>(v));

        toml::get<local_time_type>(v).hour = 9;
        t.hour = 9;
        CHECK_EQ(t, toml::get<local_time_type>(v));

        local_time_type x = toml::get<local_time_type>(std::move(v));
        CHECK_EQ(t, x);
    }
    {
        local_datetime_type dt(toml::local_date(2018, toml::month_t::Apr, 22),
                               toml::local_time(12, 30, 45));
        value_type v(dt);
        CHECK_EQ(dt, toml::get<local_datetime_type>(v));

        toml::get<local_datetime_type>(v).date.year = 2017;
        dt.date.year = 2017;
        CHECK_EQ(dt, toml::get<local_datetime_type>(v));

        toml::local_datetime x = toml::get<local_datetime_type>(std::move(v));
        CHECK_EQ(dt, x);
    }
    {
        offset_datetime_type dt(toml::local_datetime(
                    toml::local_date(2018, toml::month_t::Apr, 22),
                    toml::local_time(12, 30, 45)), toml::time_offset(9, 0));
        value_type v(dt);
        CHECK_EQ(dt, toml::get<offset_datetime_type>(v));

        toml::get<toml::offset_datetime>(v).date.year = 2017;
        dt.date.year = 2017;
        CHECK_EQ(dt, toml::get<offset_datetime_type>(v));

        offset_datetime_type x = toml::get<offset_datetime_type>(std::move(v));
        CHECK_EQ(dt, x);
    }
    {
        array_type vec;
        vec.push_back(value_type(42));
        vec.push_back(value_type(54));
        value_type v(vec);
        CHECK_EQ(vec, toml::get<array_type>(v));

        toml::get<array_type>(v).push_back(value_type(123));
        vec.push_back(value_type(123));
        CHECK_EQ(vec, toml::get<array_type>(v));

        array_type x = toml::get<array_type>(std::move(v));
        CHECK_EQ(vec, x);
    }
    {
        table_type tab;
        tab["key1"] = value_type(42);
        tab["key2"] = value_type(3.14);
        value_type v(tab);
        CHECK_EQ(tab, toml::get<table_type>(v));

        toml::get<table_type>(v)["key3"] = value_type(123);
        tab["key3"] = value_type(123);
        CHECK_EQ(tab, toml::get<table_type>(v));

        table_type x = toml::get<table_type>(std::move(v));
        CHECK_EQ(tab, x);
    }
    {
        value_type v1(42);
        CHECK_EQ(v1, toml::get<value_type>(v1));

        value_type v2(54);
        toml::get<value_type>(v1) = v2;
        CHECK_EQ(v2, toml::get<value_type>(v1));

        value_type x = toml::get<value_type>(std::move(v1));
        CHECK_EQ(v2, x);
    }
}

TEST_CASE("testing toml::get<integer-like>")
{
    using value_type = toml::value;
    {
        value_type v(42);

        CHECK_EQ(int(42),           toml::get<int          >(v));
        CHECK_EQ(short(42),         toml::get<short        >(v));
        CHECK_EQ(char(42),          toml::get<char         >(v));
        CHECK_EQ(unsigned(42),      toml::get<unsigned     >(v));
        CHECK_EQ(long(42),          toml::get<long         >(v));
        CHECK_EQ(std::int64_t(42),  toml::get<std::int64_t >(v));
        CHECK_EQ(std::uint64_t(42), toml::get<std::uint64_t>(v));
        CHECK_EQ(std::int16_t(42),  toml::get<std::int16_t >(v));
        CHECK_EQ(std::uint16_t(42), toml::get<std::uint16_t>(v));

        CHECK_EQ(int(42),           toml::get<int          >(as_const(v)));
        CHECK_EQ(short(42),         toml::get<short        >(as_const(v)));
        CHECK_EQ(char(42),          toml::get<char         >(as_const(v)));
        CHECK_EQ(unsigned(42),      toml::get<unsigned     >(as_const(v)));
        CHECK_EQ(long(42),          toml::get<long         >(as_const(v)));
        CHECK_EQ(std::int64_t(42),  toml::get<std::int64_t >(as_const(v)));
        CHECK_EQ(std::uint64_t(42), toml::get<std::uint64_t>(as_const(v)));
        CHECK_EQ(std::int16_t(42),  toml::get<std::int16_t >(as_const(v)));
        CHECK_EQ(std::uint16_t(42), toml::get<std::uint16_t>(as_const(v)));

        value_type v1(v);
        value_type v2(v);
        value_type v3(v);
        value_type v4(v);
        value_type v5(v);
        value_type v6(v);
        value_type v7(v);
        value_type v8(v);
        value_type v9(v);

        CHECK_EQ(int(42),           toml::get<int          >(v1));
        CHECK_EQ(short(42),         toml::get<short        >(v2));
        CHECK_EQ(char(42),          toml::get<char         >(v3));
        CHECK_EQ(unsigned(42),      toml::get<unsigned     >(v4));
        CHECK_EQ(long(42),          toml::get<long         >(v5));
        CHECK_EQ(std::int64_t(42),  toml::get<std::int64_t >(v6));
        CHECK_EQ(std::uint64_t(42), toml::get<std::uint64_t>(v7));
        CHECK_EQ(std::int16_t(42),  toml::get<std::int16_t >(v8));
        CHECK_EQ(std::uint16_t(42), toml::get<std::uint16_t>(v9));
    }
}

TEST_CASE("testing toml::get<floating-like>")
{
    using value_type = toml::value;
    {
        const double ref(3.14);

        value_type v(ref);
        CHECK_EQ(static_cast<float      >(ref), toml::get<float      >(v));
        CHECK_EQ(                         ref , toml::get<double     >(v));
        CHECK_EQ(static_cast<long double>(ref), toml::get<long double>(v));

        value_type v1(ref);
        value_type v2(ref);
        value_type v3(ref);
        CHECK_EQ(static_cast<float      >(ref), toml::get<float      >(std::move(v1)));
        CHECK_EQ(                         ref , toml::get<double     >(std::move(v2)));
        CHECK_EQ(static_cast<long double>(ref), toml::get<long double>(std::move(v3)));
    }
}

#if defined(TOML11_HAS_CHAR8_T)
TEST_CASE("testing toml::get<string-like>")
{
    using value_type = toml::value;
    {
        value_type v("foo");
        CHECK_EQ(u8"foo", toml::get<std::u8string>(v));
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

TEST_CASE("testing toml::get<array-like>")
{
    using value_type = toml::value;
    {
        const value_type v(toml::array{42, 54, 69, 72});

        const std::vector<int>         vec = toml::get<std::vector<int>>(v);
        const std::list<short>         lst = toml::get<std::list<short>>(v);
        const std::deque<std::int64_t> deq = toml::get<std::deque<std::int64_t>>(v);

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

        std::array<int, 4> ary = toml::get<std::array<int, 4>>(v);
        CHECK_EQ(42, ary.at(0));
        CHECK_EQ(54, ary.at(1));
        CHECK_EQ(69, ary.at(2));
        CHECK_EQ(72, ary.at(3));

        std::tuple<int, short, unsigned, long> tpl =
            toml::get<std::tuple<int, short, unsigned, long>>(v);
        CHECK_EQ(                      42 , std::get<0>(tpl));
        CHECK_EQ(static_cast<short   >(54), std::get<1>(tpl));
        CHECK_EQ(static_cast<unsigned>(69), std::get<2>(tpl));
        CHECK_EQ(static_cast<long    >(72), std::get<3>(tpl));

        const value_type p(toml::array{3.14, 2.71});
        std::pair<double, double> pr = toml::get<std::pair<double, double> >(p);
        CHECK_EQ(3.14, pr.first);
        CHECK_EQ(2.71, pr.second);
    }

    {
        value_type v(toml::array{42, 54, 69, 72});
        const std::vector<int> vec = toml::get<std::vector<int>>(std::move(v));
        CHECK_EQ(42, vec.at(0));
        CHECK_EQ(54, vec.at(1));
        CHECK_EQ(69, vec.at(2));
        CHECK_EQ(72, vec.at(3));
    }
    {
        value_type v(toml::array{42, 54, 69, 72});
        const std::deque<int> deq = toml::get<std::deque<int>>(std::move(v));
        CHECK_EQ(42, deq.at(0));
        CHECK_EQ(54, deq.at(1));
        CHECK_EQ(69, deq.at(2));
        CHECK_EQ(72, deq.at(3));
    }
    {
        value_type v(toml::array{42, 54, 69, 72});
        const std::list<int> lst = toml::get<std::list<int>>(std::move(v));
        std::list<int>::const_iterator iter = lst.begin();
        CHECK_EQ(42, *(iter++));
        CHECK_EQ(54, *(iter++));
        CHECK_EQ(69, *(iter++));
        CHECK_EQ(72, *(iter++));
    }
    {
        value_type v(toml::array{42, 54, 69, 72});
        std::array<int, 4> ary = toml::get<std::array<int, 4>>(std::move(v));
        CHECK_EQ(42, ary.at(0));
        CHECK_EQ(54, ary.at(1));
        CHECK_EQ(69, ary.at(2));
        CHECK_EQ(72, ary.at(3));
    }
    {
        value_type v(toml::array{42, 54, 69, 72});
        std::tuple<int, short, unsigned, long> tpl =
            toml::get<std::tuple<int, short, unsigned, long>>(std::move(v));
        CHECK_EQ(                      42 , std::get<0>(tpl));
        CHECK_EQ(static_cast<short   >(54), std::get<1>(tpl));
        CHECK_EQ(static_cast<unsigned>(69), std::get<2>(tpl));
        CHECK_EQ(static_cast<long    >(72), std::get<3>(tpl));
    }
}

TEST_CASE("testing toml::get<array-of-arrays>")
{
    using value_type = toml::value;
    {
        const value_type v1(toml::array{42, 54, 69, 72});
        const value_type v2(toml::array{"foo", "bar", "baz"});
        const value_type v (toml::array{v1, v2});

        std::pair<std::vector<int>, std::vector<std::string>> p =
            toml::get<std::pair<std::vector<int>, std::vector<std::string>>>(v);

        CHECK_EQ(p.first.size(), 4u);
        CHECK_EQ(p.first.at(0), 42);
        CHECK_EQ(p.first.at(1), 54);
        CHECK_EQ(p.first.at(2), 69);
        CHECK_EQ(p.first.at(3), 72);

        CHECK_EQ(p.second.size(), 3u);
        CHECK_EQ(p.second.at(0), "foo");
        CHECK_EQ(p.second.at(1), "bar");
        CHECK_EQ(p.second.at(2), "baz");

        std::tuple<std::vector<int>, std::vector<std::string>> t =
            toml::get<std::tuple<std::vector<int>, std::vector<std::string>>>(v);

        CHECK_EQ(std::get<0>(t).at(0), 42);
        CHECK_EQ(std::get<0>(t).at(1), 54);
        CHECK_EQ(std::get<0>(t).at(2), 69);
        CHECK_EQ(std::get<0>(t).at(3), 72);

        CHECK_EQ(std::get<1>(t).at(0), "foo");
        CHECK_EQ(std::get<1>(t).at(1), "bar");
        CHECK_EQ(std::get<1>(t).at(2), "baz");
    }
    {
        const value_type v1(toml::array{42, 54, 69, 72});
        const value_type v2(toml::array{"foo", "bar", "baz"});
        value_type       v (toml::array{v1, v2});

        std::pair<std::vector<int>, std::vector<std::string>> p =
            toml::get<std::pair<std::vector<int>, std::vector<std::string>>>(std::move(v));

        CHECK_EQ(p.first.size(), 4u);
        CHECK_EQ(p.first.at(0), 42);
        CHECK_EQ(p.first.at(1), 54);
        CHECK_EQ(p.first.at(2), 69);
        CHECK_EQ(p.first.at(3), 72);

        CHECK_EQ(p.second.size(), 3u);
        CHECK_EQ(p.second.at(0), "foo");
        CHECK_EQ(p.second.at(1), "bar");
        CHECK_EQ(p.second.at(2), "baz");
    }
}

TEST_CASE("testing toml::get<table-like>")
{
    using value_type = toml::value;
    {
        const value_type v1(toml::table{
                {"key1", 1},
                {"key2", 2},
                {"key3", 3},
                {"key4", 4}
            });

        const auto v = toml::get<std::map<std::string, int>>(v1);
        CHECK_EQ(v.at("key1"), 1);
        CHECK_EQ(v.at("key2"), 2);
        CHECK_EQ(v.at("key3"), 3);
        CHECK_EQ(v.at("key4"), 4);
    }
    {
        value_type v1(toml::table{
                {"key1", 1},
                {"key2", 2},
                {"key3", 3},
                {"key4", 4}
            });
        const auto v = toml::get<std::map<std::string, int>>(std::move(v1));
        CHECK_EQ(v.at("key1"), 1);
        CHECK_EQ(v.at("key2"), 2);
        CHECK_EQ(v.at("key3"), 3);
        CHECK_EQ(v.at("key4"), 4);
    }
}

TEST_CASE("testing toml::get<time_point>(local_date)")
{
    using value_type = toml::value;

    value_type v1(toml::local_date{2018, toml::month_t::Apr, 1});
    const auto date = std::chrono::system_clock::to_time_t(
            toml::get<std::chrono::system_clock::time_point>(v1));

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

TEST_CASE("testing toml::get<duration>")
{
    using value_type = toml::value;

    value_type v1(toml::local_time{12, 30, 45});
    const auto time = toml::get<std::chrono::seconds>(v1);
    const bool result = time == std::chrono::hours(12)   +
                                std::chrono::minutes(30) +
                                std::chrono::seconds(45);
    CHECK_UNARY(result);
}

TEST_CASE("testing toml::get<time_point>(local_datetime)")
{
    using value_type = toml::value;

    value_type v1(toml::local_datetime(
                toml::local_date{2018, toml::month_t::Apr, 1},
                toml::local_time{12, 30, 45}));

    const auto date = std::chrono::system_clock::to_time_t(
            toml::get<std::chrono::system_clock::time_point>(v1));

    std::tm t;
    t.tm_year = 2018 - 1900;
    t.tm_mon  = 4    - 1;
    t.tm_mday = 1;
    t.tm_hour = 12;
    t.tm_min  = 30;
    t.tm_sec  = 45;
    t.tm_isdst = -1;
    const auto c = std::mktime(&t);
    CHECK_EQ(c, date);
}

TEST_CASE("testing toml::get<time_point>(offset_datetime)")
{
    using value_type = toml::value;
    {
        value_type v1(toml::offset_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 0},
                    toml::time_offset{9, 0}));
        //    2018-04-01T12:30:00+09:00
        // == 2018-04-01T03:30:00Z

        const auto date = toml::get<std::chrono::system_clock::time_point>(v1);
        const auto timet = std::chrono::system_clock::to_time_t(date);

        // get time_t as gmtime (2018-04-01T03:30:00Z)
        const auto tm = toml::detail::gmtime_s(std::addressof(timet));
        CHECK_EQ(tm.tm_year + 1900, 2018);
        CHECK_EQ(tm.tm_mon  + 1,       4);
        CHECK_EQ(tm.tm_mday,           1);
        CHECK_EQ(tm.tm_hour,           3);
        CHECK_EQ(tm.tm_min,           30);
        CHECK_EQ(tm.tm_sec,            0);
    }

    {
        value_type v1(toml::offset_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 0},
                    toml::time_offset{-8, 0}));
        //    2018-04-01T12:30:00-08:00
        //, 2018-04-01T20:30:00Z

        const auto date = toml::get<std::chrono::system_clock::time_point>(v1);
        const auto timet = std::chrono::system_clock::to_time_t(date);

        // get time_t as gmtime (2018-04-01T03:30:00Z)
        const auto tm = toml::detail::gmtime_s(std::addressof(timet));
        CHECK_EQ(tm.tm_year + 1900, 2018);
        CHECK_EQ(tm.tm_mon  + 1,       4);
        CHECK_EQ(tm.tm_mday,           1);
        CHECK_EQ(tm.tm_hour,          20);
        CHECK_EQ(tm.tm_min,           30);
        CHECK_EQ(tm.tm_sec,            0);
    }
}

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/datetime.hpp>

TEST_CASE("testing local date")
{
    const toml::local_date date(2018, toml::month_t::Jan, 1);
    const toml::local_date date1(date);
    CHECK(date == date1);

    const std::chrono::system_clock::time_point tp(date);
    const toml::local_date date2(tp);
    CHECK(date == date2);

    const toml::local_date date3(2017, toml::month_t::Dec, 31);
    CHECK(date > date3);

    std::ostringstream oss;
    oss << date;
    CHECK(oss.str() == std::string("2018-01-01"));
}

TEST_CASE("testing local time")
{
    const toml::local_time time(12, 30, 45);
    const toml::local_time time1(time);
    CHECK(time == time1);

    const std::chrono::nanoseconds dur(time);
    std::chrono::nanoseconds ns(0);
    ns += std::chrono::hours  (12);
    ns += std::chrono::minutes(30);
    ns += std::chrono::seconds(45);
    CHECK(dur.count() == ns.count());

    const toml::local_time time3(12, 15, 45);
    CHECK(time > time3);

    {
        std::ostringstream oss;
        oss << time;
        CHECK(oss.str() == std::string("12:30:45"));
    }

    {
        const toml::local_time time4(12, 30, 45, 123, 456);
        std::ostringstream oss;
        oss << time4;
        CHECK(oss.str() == std::string("12:30:45.123456"));
    }
}

TEST_CASE("testing time offset")
{
    const toml::time_offset time(9, 30);
    const toml::time_offset time1(time);
    CHECK(time == time1);

    const std::chrono::minutes dur(time);
    std::chrono::minutes m(0);
    m += std::chrono::hours  (9);
    m += std::chrono::minutes(30);
    CHECK(dur.count() == m.count());

    const toml::time_offset time2(9, 0);
    CHECK(time2 < time);

    std::ostringstream oss;
    oss << time;
    CHECK(oss.str() == std::string("+09:30"));
}

TEST_CASE("testing local datetime")
{
    const toml::local_datetime dt(toml::local_date(2018, toml::month_t::Jan, 1),
                                  toml::local_time(12, 30, 45));
    const toml::local_datetime dt1(dt);
    CHECK(dt == dt1);

    const std::chrono::system_clock::time_point tp(dt);
    const toml::local_datetime dt2(tp);
    CHECK(dt == dt2);

    std::ostringstream oss;
    oss << dt;
    CHECK(oss.str() == std::string("2018-01-01T12:30:45"));
}

TEST_CASE("testing offset datetime")
{
    const toml::offset_datetime dt(toml::local_date(2018, toml::month_t::Jan, 1),
                                   toml::local_time(12, 30, 45),
                                   toml::time_offset(9, 30));
    const toml::offset_datetime dt1(dt);
    CHECK(dt == dt1);

    const std::chrono::system_clock::time_point tp1(dt);
    const toml::offset_datetime dt2(tp1);
    const std::chrono::system_clock::time_point tp2(dt2);
    const bool tp_same = (tp1 == tp2);
    CHECK(tp_same);

    {
        std::ostringstream oss;
        oss << dt;
        CHECK(oss.str() == std::string("2018-01-01T12:30:45+09:30"));
    }
    {
        const toml::offset_datetime dt3(
                toml::local_date(2018, toml::month_t::Jan, 1),
                toml::local_time(12, 30, 45),
                toml::time_offset(0, 0));
        std::ostringstream oss;
        oss << dt3;
        CHECK(oss.str() == std::string("2018-01-01T12:30:45Z"));
    }
}

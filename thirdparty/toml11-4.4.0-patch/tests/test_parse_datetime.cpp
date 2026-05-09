#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/parser.hpp>

TEST_CASE("testing offset_datetime")
{
    const auto fmt = [](toml::datetime_delimiter_kind d, bool has_sec, std::size_t prec) {
        toml::offset_datetime_format_info f;
        f.delimiter = d;
        f.has_seconds = has_sec;
        f.subsecond_precision = prec;
        return f;
    };

    {
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27T07:32:00Z",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0)
                ),
                toml::time_offset(0, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, true, 0), ctx);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27T07:32:00-07:00",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0)
                ),
                toml::time_offset(-7, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, true, 0), ctx);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27T07:32:00.999999-07:00",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0, 999, 999)
                ),
                toml::time_offset(-7, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, true, 6), ctx);



        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27 07:32:00Z",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0)
                ),
                toml::time_offset(0, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::space, true, 0), ctx);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27 07:32:00-07:00",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0)
                ),
                toml::time_offset(-7, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::space, true, 0), ctx);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27 07:32:00.999999-07:00",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0, 999, 999)
                ),
                toml::time_offset(-7, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::space, true, 6), ctx);
    }
    {
        auto spec = toml::spec::v(1,0,0);
        spec.v1_1_0_make_seconds_optional = true;
        toml::detail::context<toml::type_config> ctx(spec);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27T07:32Z",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0)
                ),
                toml::time_offset(0, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, false, 0), ctx);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27T07:32:00Z",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0)
                ),
                toml::time_offset(0, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, true, 0), ctx);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27T07:32:00-07:00",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0)
                ),
                toml::time_offset(-7, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, true, 0), ctx);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27T07:32:00.999999-07:00",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0, 999, 999)
                ),
                toml::time_offset(-7, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, true, 6), ctx);


        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27 07:32Z",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0)
                ),
                toml::time_offset(0, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::space, false, 0), ctx);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27 07:32:00Z",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0)
                ),
                toml::time_offset(0, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::space, true, 0), ctx);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27 07:32:00-07:00",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0)
                ),
                toml::time_offset(-7, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::space, true, 0), ctx);

        toml11_test_parse_success<toml::value_t::offset_datetime>(
            "1979-05-27 07:32:00.999999-07:00",
            toml::offset_datetime(
                toml::local_datetime(
                    toml::local_date(1979, toml::month_t::May, 27),
                    toml::local_time(7, 32, 0, 999, 999)
                ),
                toml::time_offset(-7, 0)
            ),
            comments(), fmt(toml::datetime_delimiter_kind::space, true, 6), ctx);
    }
}

TEST_CASE("testing local_datetime")
{
    const auto fmt = [](toml::datetime_delimiter_kind d, bool has_sec, std::size_t prec) {
        toml::local_datetime_format_info f;
        f.delimiter = d;
        f.has_seconds = has_sec;
        f.subsecond_precision = prec;
        return f;
    };

    {
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));

        toml11_test_parse_success<toml::value_t::local_datetime>( "1979-05-27T07:32:00",
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0)),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, true, 0), ctx);
        toml11_test_parse_success<toml::value_t::local_datetime>( "1979-05-27T07:32:00.9999",
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0, 999, 900, 0)),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, true, 4), ctx);

        toml11_test_parse_success<toml::value_t::local_datetime>( "1979-05-27 07:32:00",
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0)),
            comments(), fmt(toml::datetime_delimiter_kind::space, true, 0), ctx);
        toml11_test_parse_success<toml::value_t::local_datetime>( "1979-05-27 07:32:00.9999",
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0, 999, 900, 0)),
            comments(), fmt(toml::datetime_delimiter_kind::space, true, 4), ctx);
    }
    {
        auto spec = toml::spec::v(1,0,0);
        spec.v1_1_0_make_seconds_optional = true;
        toml::detail::context<toml::type_config> ctx(spec);

        toml11_test_parse_success<toml::value_t::local_datetime>( "1979-05-27T07:32",
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0)),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, false, 0), ctx);
        toml11_test_parse_success<toml::value_t::local_datetime>( "1979-05-27T07:32:00",
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0)),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, true, 0), ctx);
        toml11_test_parse_success<toml::value_t::local_datetime>( "1979-05-27T07:32:00.9999",
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0, 999, 900, 0)),
            comments(), fmt(toml::datetime_delimiter_kind::upper_T, true, 4), ctx);

        toml11_test_parse_success<toml::value_t::local_datetime>( "1979-05-27 07:32",
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0)),
            comments(), fmt(toml::datetime_delimiter_kind::space, false, 0), ctx);
        toml11_test_parse_success<toml::value_t::local_datetime>( "1979-05-27 07:32:00",
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0)),
            comments(), fmt(toml::datetime_delimiter_kind::space, true, 0), ctx);
        toml11_test_parse_success<toml::value_t::local_datetime>( "1979-05-27 07:32:00.9999",
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0, 999, 900, 0)),
            comments(), fmt(toml::datetime_delimiter_kind::space, true, 4), ctx);
    }
}

TEST_CASE("testing local_date")
{
    toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));

    toml11_test_parse_success<toml::value_t::local_date>("1979-05-27", toml::local_date(1979, toml::month_t::May, 27), comments(), toml::local_date_format_info{}, ctx);
    toml11_test_parse_success<toml::value_t::local_date>("0979-12-27", toml::local_date( 979, toml::month_t::Dec, 27), comments(), toml::local_date_format_info{}, ctx);

    toml11_test_parse_failure(toml::detail::parse_local_date<toml::type_config>, "1979-00-27", ctx);
    toml11_test_parse_failure(toml::detail::parse_local_date<toml::type_config>, "1979-13-27", ctx);
    toml11_test_parse_failure(toml::detail::parse_local_date<toml::type_config>, "1979-05-00", ctx);
    toml11_test_parse_failure(toml::detail::parse_local_date<toml::type_config>, "1979-05-32", ctx);

    toml11_test_parse_failure(toml::detail::parse_local_date<toml::type_config>, "1979-05-2",   ctx);
    toml11_test_parse_failure(toml::detail::parse_local_date<toml::type_config>, "1979-5-02",   ctx);
    toml11_test_parse_failure(toml::detail::parse_local_date<toml::type_config>, "1979-5-2",    ctx);
    toml11_test_parse_failure(toml::detail::parse_local_date<toml::type_config>, "979-5-2",     ctx);
    toml11_test_parse_failure(toml::detail::parse_local_date<toml::type_config>, "12345-05-27", ctx);
}

TEST_CASE("testing local_time")
{
    const auto fmt = [](const bool has_sec, const std::size_t sec_prec) {
        toml::local_time_format_info f;
        f.has_seconds = has_sec;
        f.subsecond_precision = sec_prec;
        return f;

    };
    {
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1,0,0));

        toml11_test_parse_success<toml::value_t::local_time>("01:02:03",           toml::local_time(1,  2,  3),                comments(), fmt(true, 0), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45",           toml::local_time(1, 23, 45),                comments(), fmt(true, 0), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.1",         toml::local_time(1, 23, 45, 100),           comments(), fmt(true, 1), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.12",        toml::local_time(1, 23, 45, 120),           comments(), fmt(true, 2), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.123",       toml::local_time(1, 23, 45, 123),           comments(), fmt(true, 3), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.1234",      toml::local_time(1, 23, 45, 123, 400),      comments(), fmt(true, 4), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.1234567",   toml::local_time(1, 23, 45, 123, 456, 700), comments(), fmt(true, 7), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.123456789", toml::local_time(1, 23, 45, 123, 456, 789), comments(), fmt(true, 9), ctx);
    }

    {
        auto spec = toml::spec::v(1,0,0);
        spec.v1_1_0_make_seconds_optional = true;

        toml::detail::context<toml::type_config> ctx(spec);

        toml11_test_parse_success<toml::value_t::local_time>("01:23",              toml::local_time(1, 23,  0),                comments(), fmt(false, 0), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:02",              toml::local_time(1,  2,  0),                comments(), fmt(false, 0), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:02:03",           toml::local_time(1,  2,  3),                comments(), fmt(true,  0), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45",           toml::local_time(1, 23, 45),                comments(), fmt(true,  0), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.1",         toml::local_time(1, 23, 45, 100),           comments(), fmt(true,  1), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.12",        toml::local_time(1, 23, 45, 120),           comments(), fmt(true,  2), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.123",       toml::local_time(1, 23, 45, 123),           comments(), fmt(true,  3), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.1234",      toml::local_time(1, 23, 45, 123, 400),      comments(), fmt(true,  4), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.1234567",   toml::local_time(1, 23, 45, 123, 456, 700), comments(), fmt(true,  7), ctx);
        toml11_test_parse_success<toml::value_t::local_time>("01:23:45.123456789", toml::local_time(1, 23, 45, 123, 456, 789), comments(), fmt(true,  9), ctx);
    }
}

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

#define TOML11_TEST_GET_OR_EXACT(ty, init_expr, opt_expr)\
    {                                                    \
        const ty init init_expr ;                        \
        const ty opt  opt_expr ;                         \
        const value_type v(init);                        \
        CHECK_NE(init, opt);                             \
        CHECK_EQ(init, toml::get_or(v, opt));            \
    }

TEST_CASE("testing get_or with exact types")
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

    TOML11_TEST_GET_OR_EXACT(boolean_type,         ( true), (false))
    TOML11_TEST_GET_OR_EXACT(integer_type,         (   42), (   54))
    TOML11_TEST_GET_OR_EXACT(floating_type,        ( 3.14), ( 2.71))
    TOML11_TEST_GET_OR_EXACT(string_type,          ("foo"), ("bar"))
    TOML11_TEST_GET_OR_EXACT(local_time_type,      (12, 30, 45), (6, 0, 30))
    TOML11_TEST_GET_OR_EXACT(local_date_type,      (2019, toml::month_t::Apr, 1),
                                                   (1999, toml::month_t::Jan, 2))
    TOML11_TEST_GET_OR_EXACT(local_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30))
            )
    TOML11_TEST_GET_OR_EXACT(offset_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45), toml::time_offset( 9, 0)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30), toml::time_offset(-3, 0))
            )

    TOML11_TEST_GET_OR_EXACT(array_type, ({1,2,3,4,5}), ({6,7,8,9,10}));
    TOML11_TEST_GET_OR_EXACT(table_type, ({{"key1", 42}, {"key2", "foo"}}),
                                         ({{"key1", 54}, {"key2", "bar"}}));
}
#undef TOML11_TEST_GET_OR_EXACT

#define TOML11_TEST_GET_OR_MOVE_EXACT(ty, init_expr, opt_expr)       \
    {                                                                \
        const ty init init_expr ;                                    \
        ty       opt  opt_expr ;                                     \
        value_type v(init);                                          \
        CHECK_NE(init, opt);                                         \
        const auto opt_ = toml::get_or(std::move(v), std::move(opt));\
        CHECK_EQ(init, opt_);                                        \
    }

TEST_CASE("testing toml::get_or with moved argument")
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

    TOML11_TEST_GET_OR_MOVE_EXACT(boolean_type,         ( true), (false))
    TOML11_TEST_GET_OR_MOVE_EXACT(integer_type,         (   42), (   54))
    TOML11_TEST_GET_OR_MOVE_EXACT(floating_type,        ( 3.14), ( 2.71))
    TOML11_TEST_GET_OR_MOVE_EXACT(string_type,          ("foo"), ("bar"))
    TOML11_TEST_GET_OR_MOVE_EXACT(local_time_type,      (12, 30, 45), (6, 0, 30))
    TOML11_TEST_GET_OR_MOVE_EXACT(local_date_type,      (2019, toml::month_t::Apr, 1),
                                                        (1999, toml::month_t::Jan, 2))
    TOML11_TEST_GET_OR_MOVE_EXACT(local_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30))
        )
    TOML11_TEST_GET_OR_MOVE_EXACT(offset_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45), toml::time_offset( 9, 0)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30), toml::time_offset(-3, 0))
        )

    TOML11_TEST_GET_OR_MOVE_EXACT(array_type, ({1,2,3,4,5}), ({6,7,8,9,10}));
    TOML11_TEST_GET_OR_MOVE_EXACT(table_type, ({{"key1", 42}, {"key2", "foo"}}),
                                              ({{"key1", 54}, {"key2", "bar"}}));

}
#undef TOML11_TEST_GET_OR_MOVE_EXACT


#define TOML11_TEST_GET_OR_MODIFY(ty, init_expr, opt_expr)\
    {                                                     \
        const ty init init_expr ;                         \
              ty opt1 opt_expr ;                          \
              ty opt2 opt_expr ;                          \
        value_type v(init);                               \
        CHECK_NE(init, opt1);                             \
        toml::get_or(v, opt2) = opt1;                     \
        CHECK_EQ(opt1, toml::get<ty>(v));                 \
    }

TEST_CASE("testing if get_or can modify value")
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

    TOML11_TEST_GET_OR_MODIFY(boolean_type,         ( true), (false))
    TOML11_TEST_GET_OR_MODIFY(integer_type,         (   42), (   54))
    TOML11_TEST_GET_OR_MODIFY(floating_type,        ( 3.14), ( 2.71))
    TOML11_TEST_GET_OR_MODIFY(string_type,          ("foo"), ("bar"))
    TOML11_TEST_GET_OR_MODIFY(local_time_type,      (12, 30, 45), (6, 0, 30))
    TOML11_TEST_GET_OR_MODIFY(local_date_type,      (2019, toml::month_t::Apr, 1),
                                                    (1999, toml::month_t::Jan, 2))
    TOML11_TEST_GET_OR_MODIFY(local_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30))
        )
    TOML11_TEST_GET_OR_MODIFY(offset_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45), toml::time_offset( 9, 0)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30), toml::time_offset(-3, 0))
        )

    TOML11_TEST_GET_OR_MODIFY(array_type, ({1,2,3,4,5}), ({6,7,8,9,10}));
    TOML11_TEST_GET_OR_MODIFY(table_type, ({{"key1", 42}, {"key2", "foo"}}),
                                          ({{"key1", 54}, {"key2", "bar"}}));
}
#undef TOML11_TEST_GET_OR_MODIFY

#define TOML11_TEST_GET_OR_FALLBACK(init_type, opt_type)  \
    {                                                     \
        value_type v(init_type);                          \
        CHECK_EQ(opt_type, toml::get_or(v, opt_type));    \
    }

TEST_CASE("testing get_or can return optional value on failure")
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

    const boolean_type         boolean        (true);
    const integer_type         integer        (42);
    const floating_type        floating       (3.14);
    const string_type          string         ("foo");
    const local_time_type      local_time     (12, 30, 45);
    const local_date_type      local_date     (2019, toml::month_t::Apr, 1);
    const local_datetime_type  local_datetime (
            toml::local_date(2019, toml::month_t::Apr, 1),
            toml::local_time(12, 30, 45)
        );
    const offset_datetime_type offset_datetime(
            toml::local_date(2019, toml::month_t::Apr, 1),
            toml::local_time(12, 30, 45), toml::time_offset( 9, 0));

    const array_type array{1, 2, 3, 4, 5};
    const table_type table{{"key1", 42}, {"key2", "foo"}};

    TOML11_TEST_GET_OR_FALLBACK(boolean, integer        );
    TOML11_TEST_GET_OR_FALLBACK(boolean, floating       );
    TOML11_TEST_GET_OR_FALLBACK(boolean, string         );
    TOML11_TEST_GET_OR_FALLBACK(boolean, local_time     );
    TOML11_TEST_GET_OR_FALLBACK(boolean, local_date     );
    TOML11_TEST_GET_OR_FALLBACK(boolean, local_datetime );
    TOML11_TEST_GET_OR_FALLBACK(boolean, offset_datetime);
    TOML11_TEST_GET_OR_FALLBACK(boolean, array          );
    TOML11_TEST_GET_OR_FALLBACK(boolean, table          );

    TOML11_TEST_GET_OR_FALLBACK(integer, boolean        );
    TOML11_TEST_GET_OR_FALLBACK(integer, floating       );
    TOML11_TEST_GET_OR_FALLBACK(integer, string         );
    TOML11_TEST_GET_OR_FALLBACK(integer, local_time     );
    TOML11_TEST_GET_OR_FALLBACK(integer, local_date     );
    TOML11_TEST_GET_OR_FALLBACK(integer, local_datetime );
    TOML11_TEST_GET_OR_FALLBACK(integer, offset_datetime);
    TOML11_TEST_GET_OR_FALLBACK(integer, array          );
    TOML11_TEST_GET_OR_FALLBACK(integer, table          );

    TOML11_TEST_GET_OR_FALLBACK(floating, boolean        );
    TOML11_TEST_GET_OR_FALLBACK(floating, integer        );
    TOML11_TEST_GET_OR_FALLBACK(floating, string         );
    TOML11_TEST_GET_OR_FALLBACK(floating, local_time     );
    TOML11_TEST_GET_OR_FALLBACK(floating, local_date     );
    TOML11_TEST_GET_OR_FALLBACK(floating, local_datetime );
    TOML11_TEST_GET_OR_FALLBACK(floating, offset_datetime);
    TOML11_TEST_GET_OR_FALLBACK(floating, array          );
    TOML11_TEST_GET_OR_FALLBACK(floating, table          );

    TOML11_TEST_GET_OR_FALLBACK(string, boolean        );
    TOML11_TEST_GET_OR_FALLBACK(string, integer        );
    TOML11_TEST_GET_OR_FALLBACK(string, floating       );
    TOML11_TEST_GET_OR_FALLBACK(string, local_time     );
    TOML11_TEST_GET_OR_FALLBACK(string, local_date     );
    TOML11_TEST_GET_OR_FALLBACK(string, local_datetime );
    TOML11_TEST_GET_OR_FALLBACK(string, offset_datetime);
    TOML11_TEST_GET_OR_FALLBACK(string, array          );
    TOML11_TEST_GET_OR_FALLBACK(string, table          );

    TOML11_TEST_GET_OR_FALLBACK(local_time, boolean        );
    TOML11_TEST_GET_OR_FALLBACK(local_time, integer        );
    TOML11_TEST_GET_OR_FALLBACK(local_time, floating       );
    TOML11_TEST_GET_OR_FALLBACK(local_time, string         );
    TOML11_TEST_GET_OR_FALLBACK(local_time, local_date     );
    TOML11_TEST_GET_OR_FALLBACK(local_time, local_datetime );
    TOML11_TEST_GET_OR_FALLBACK(local_time, offset_datetime);
    TOML11_TEST_GET_OR_FALLBACK(local_time, array          );
    TOML11_TEST_GET_OR_FALLBACK(local_time, table          );

    TOML11_TEST_GET_OR_FALLBACK(local_date, boolean        );
    TOML11_TEST_GET_OR_FALLBACK(local_date, integer        );
    TOML11_TEST_GET_OR_FALLBACK(local_date, floating       );
    TOML11_TEST_GET_OR_FALLBACK(local_date, string         );
    TOML11_TEST_GET_OR_FALLBACK(local_date, local_time     );
    TOML11_TEST_GET_OR_FALLBACK(local_date, local_datetime );
    TOML11_TEST_GET_OR_FALLBACK(local_date, offset_datetime);
    TOML11_TEST_GET_OR_FALLBACK(local_date, array          );
    TOML11_TEST_GET_OR_FALLBACK(local_date, table          );

    TOML11_TEST_GET_OR_FALLBACK(local_datetime, boolean        );
    TOML11_TEST_GET_OR_FALLBACK(local_datetime, integer        );
    TOML11_TEST_GET_OR_FALLBACK(local_datetime, floating       );
    TOML11_TEST_GET_OR_FALLBACK(local_datetime, string         );
    TOML11_TEST_GET_OR_FALLBACK(local_datetime, local_time     );
    TOML11_TEST_GET_OR_FALLBACK(local_datetime, local_date     );
    TOML11_TEST_GET_OR_FALLBACK(local_datetime, offset_datetime);
    TOML11_TEST_GET_OR_FALLBACK(local_datetime, array          );
    TOML11_TEST_GET_OR_FALLBACK(local_datetime, table          );

    TOML11_TEST_GET_OR_FALLBACK(offset_datetime, boolean        );
    TOML11_TEST_GET_OR_FALLBACK(offset_datetime, integer        );
    TOML11_TEST_GET_OR_FALLBACK(offset_datetime, floating       );
    TOML11_TEST_GET_OR_FALLBACK(offset_datetime, string         );
    TOML11_TEST_GET_OR_FALLBACK(offset_datetime, local_time     );
    TOML11_TEST_GET_OR_FALLBACK(offset_datetime, local_date     );
    TOML11_TEST_GET_OR_FALLBACK(offset_datetime, local_datetime );
    TOML11_TEST_GET_OR_FALLBACK(offset_datetime, array          );
    TOML11_TEST_GET_OR_FALLBACK(offset_datetime, table          );

    TOML11_TEST_GET_OR_FALLBACK(array, boolean        );
    TOML11_TEST_GET_OR_FALLBACK(array, integer        );
    TOML11_TEST_GET_OR_FALLBACK(array, floating       );
    TOML11_TEST_GET_OR_FALLBACK(array, string         );
    TOML11_TEST_GET_OR_FALLBACK(array, local_time     );
    TOML11_TEST_GET_OR_FALLBACK(array, local_date     );
    TOML11_TEST_GET_OR_FALLBACK(array, local_datetime );
    TOML11_TEST_GET_OR_FALLBACK(array, offset_datetime);
    TOML11_TEST_GET_OR_FALLBACK(array, table          );

    TOML11_TEST_GET_OR_FALLBACK(table, boolean        );
    TOML11_TEST_GET_OR_FALLBACK(table, integer        );
    TOML11_TEST_GET_OR_FALLBACK(table, floating       );
    TOML11_TEST_GET_OR_FALLBACK(table, string         );
    TOML11_TEST_GET_OR_FALLBACK(table, local_time     );
    TOML11_TEST_GET_OR_FALLBACK(table, local_date     );
    TOML11_TEST_GET_OR_FALLBACK(table, local_datetime );
    TOML11_TEST_GET_OR_FALLBACK(table, offset_datetime);
    TOML11_TEST_GET_OR_FALLBACK(table, array          );
}
#undef TOML11_TEST_GET_OR_FALLBACK

TEST_CASE("testing get_or with optional integer")
{
    {
        toml::value v1(42);
        toml::value v2(3.14);
        CHECK_EQ(42u, toml::get_or(v1, 0u));
        CHECK_EQ(0u , toml::get_or(v2, 0u));
    }
    {
        toml::value v1(42);
        toml::value v2(3.14);
        CHECK_EQ(42u, toml::get_or(std::move(v1), 0u));
        CHECK_EQ( 0u, toml::get_or(std::move(v2), 0u));
    }
}

TEST_CASE("testing get_or with optional float")
{
    {
        toml::value v1(42);
        toml::value v2(3.14);
        CHECK_EQ(2.71f, toml::get_or(v1, 2.71f));
        CHECK_EQ(static_cast<float>(v2.as_floating()), toml::get_or(v2, 2.71f));
    }
    {
        toml::value v1(42);
        toml::value v2(3.14);
        CHECK_EQ(2.71f                   , toml::get_or(std::move(v1), 2.71f));
        CHECK_EQ(static_cast<float>(3.14), toml::get_or(std::move(v2), 2.71f));
    }
}

TEST_CASE("testing get_or with optional string literal")
{
    {
        toml::value v1("foobar");
        toml::value v2(42);

        CHECK_EQ("foobar", toml::get_or(v1, "bazqux"));
        CHECK_EQ("bazqux", toml::get_or(v2, "bazqux"));
    }
    {
        toml::value v1("foobar");
        toml::value v2(42);

        CHECK_EQ("foobar", toml::get_or(std::move(v1), "bazqux"));
        CHECK_EQ("bazqux", toml::get_or(std::move(v2), "bazqux"));
    }
}

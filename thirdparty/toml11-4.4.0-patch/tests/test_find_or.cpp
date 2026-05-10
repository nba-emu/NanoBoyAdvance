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

#if TOML11_CPLUSPLUS_STANDARD_VERSION >= 201703L
#include <string_view>
#endif

#define TOML11_TEST_FIND_OR_EXACT(ty, init_expr, opt_expr)\
    {                                                     \
        const ty init init_expr ;                         \
        const ty opt  opt_expr ;                          \
        const value_type v = toml::table{{"key", init}};  \
        CHECK_NE(init, opt);                              \
        CHECK_EQ(init, toml::find_or(v, "key", opt));     \
    }

TEST_CASE("testing toml::find_or with toml types")
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

    TOML11_TEST_FIND_OR_EXACT(boolean_type,         ( true), (false))
    TOML11_TEST_FIND_OR_EXACT(integer_type,         (   42), (   54))
    TOML11_TEST_FIND_OR_EXACT(floating_type,        ( 3.14), ( 2.71))
    TOML11_TEST_FIND_OR_EXACT(string_type,          ("foo"), ("bar"))
    TOML11_TEST_FIND_OR_EXACT(local_time_type,      (12, 30, 45), (6, 0, 30))
    TOML11_TEST_FIND_OR_EXACT(local_date_type,      (2019, toml::month_t::Apr, 1),
                                                    (1999, toml::month_t::Jan, 2))
    TOML11_TEST_FIND_OR_EXACT(local_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30))
            )
    TOML11_TEST_FIND_OR_EXACT(offset_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45), toml::time_offset( 9, 0)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30), toml::time_offset(-3, 0))
            )
    TOML11_TEST_FIND_OR_EXACT(array_type, ({1,2,3,4,5}), ({6,7,8,9,10}));
    TOML11_TEST_FIND_OR_EXACT(table_type, ({{"key1", 42}, {"key2", "foo"}}),
                                          ({{"key1", 54}, {"key2", "bar"}}));
}
#undef TOML11_TEST_FIND_OR_EXACT

#define TOML11_TEST_FIND_OR_MOVE(ty, init_expr, opt_expr)   \
    {                                                       \
        const ty init init_expr ;                           \
        ty       opt  opt_expr ;                            \
        value_type v = toml::table{{"key", init}};          \
        CHECK_NE(init, opt);                                \
        const auto moved = toml::find_or(std::move(v), "key", std::move(opt));\
        CHECK_EQ(init, moved);                              \
    }

TEST_CASE("testing toml::find_or with moved argument")
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


    TOML11_TEST_FIND_OR_MOVE(boolean_type,         ( true), (false))
    TOML11_TEST_FIND_OR_MOVE(integer_type,         (   42), (   54))
    TOML11_TEST_FIND_OR_MOVE(floating_type,        ( 3.14), ( 2.71))
    TOML11_TEST_FIND_OR_MOVE(string_type,          ("foo"), ("bar"))
    TOML11_TEST_FIND_OR_MOVE(local_time_type,      (12, 30, 45), (6, 0, 30))
    TOML11_TEST_FIND_OR_MOVE(local_date_type,      (2019, toml::month_t::Apr, 1),
                                              (1999, toml::month_t::Jan, 2))
    TOML11_TEST_FIND_OR_MOVE(local_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30))
            )
    TOML11_TEST_FIND_OR_MOVE(offset_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45), toml::time_offset( 9, 0)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30), toml::time_offset(-3, 0))
            )
    TOML11_TEST_FIND_OR_MOVE(array_type, ({1,2,3,4,5}), ({6,7,8,9,10}));
    TOML11_TEST_FIND_OR_MOVE(table_type, ({{"key1", 42}, {"key2", "foo"}}),
                                         ({{"key1", 54}, {"key2", "bar"}}));
}
#undef TOML11_TEST_FIND_OR_MOVE


#define TOML11_TEST_FIND_OR_MODIFY(ty, init_expr, opt_expr)\
    {                                                      \
        const ty init init_expr ;                          \
              ty opt1 opt_expr ;                           \
              ty opt2 opt_expr ;                           \
        value_type v = toml::table{{"key", init}};         \
        CHECK_NE(init, opt1);                              \
        toml::find_or(v, "key", opt2) = opt1;              \
        CHECK_EQ(opt1, toml::find<ty>(v, "key"));          \
    }

TEST_CASE("testing find_or with modification")
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

    TOML11_TEST_FIND_OR_MODIFY(boolean_type,         ( true), (false))
    TOML11_TEST_FIND_OR_MODIFY(integer_type,         (   42), (   54))
    TOML11_TEST_FIND_OR_MODIFY(floating_type,        ( 3.14), ( 2.71))
    TOML11_TEST_FIND_OR_MODIFY(string_type,          ("foo"), ("bar"))
    TOML11_TEST_FIND_OR_MODIFY(local_time_type,      (12, 30, 45), (6, 0, 30))
    TOML11_TEST_FIND_OR_MODIFY(local_date_type,      (2019, toml::month_t::Apr, 1),
                                              (1999, toml::month_t::Jan, 2))
    TOML11_TEST_FIND_OR_MODIFY(local_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30))
            )
    TOML11_TEST_FIND_OR_MODIFY(offset_datetime_type,
            (toml::local_date(2019, toml::month_t::Apr, 1), toml::local_time(12, 30, 45), toml::time_offset( 9, 0)),
            (toml::local_date(1999, toml::month_t::Jan, 2), toml::local_time( 6,  0, 30), toml::time_offset(-3, 0))
            )

    TOML11_TEST_FIND_OR_MODIFY(array_type, ({1,2,3,4,5}), ({6,7,8,9,10}));
    TOML11_TEST_FIND_OR_MODIFY(table_type, ({{"key1", 42}, {"key2", "foo"}}),
                                           ({{"key1", 54}, {"key2", "bar"}}));
}
#undef TOML11_TEST_FIND_OR_MODIFY

#define TOML11_TEST_FIND_OR_FALLBACK(init_type, opt_type)          \
    {                                                              \
        value_type v1(init_type);                                  \
        CHECK_EQ(opt_type, toml::find_or(v1, "key", opt_type));    \
        value_type v2 = toml::table{{"different_key", init_type}}; \
        CHECK_EQ(opt_type, toml::find_or(v2, "key", opt_type));    \
    }

TEST_CASE("testing toml::find_or returns fallback if failed")
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
            toml::local_time(12, 30, 45));
    const offset_datetime_type offset_datetime(
            toml::local_date(2019, toml::month_t::Apr, 1),
            toml::local_time(12, 30, 45), toml::time_offset( 9, 0));

    const array_type array{1, 2, 3, 4, 5};
    const table_type table{{"key1", 42}, {"key2", "foo"}};

    TOML11_TEST_FIND_OR_FALLBACK(boolean, integer        );
    TOML11_TEST_FIND_OR_FALLBACK(boolean, floating       );
    TOML11_TEST_FIND_OR_FALLBACK(boolean, string         );
    TOML11_TEST_FIND_OR_FALLBACK(boolean, local_time     );
    TOML11_TEST_FIND_OR_FALLBACK(boolean, local_date     );
    TOML11_TEST_FIND_OR_FALLBACK(boolean, local_datetime );
    TOML11_TEST_FIND_OR_FALLBACK(boolean, offset_datetime);
    TOML11_TEST_FIND_OR_FALLBACK(boolean, array          );
    TOML11_TEST_FIND_OR_FALLBACK(boolean, table          );

    TOML11_TEST_FIND_OR_FALLBACK(integer, boolean        );
    TOML11_TEST_FIND_OR_FALLBACK(integer, floating       );
    TOML11_TEST_FIND_OR_FALLBACK(integer, string         );
    TOML11_TEST_FIND_OR_FALLBACK(integer, local_time     );
    TOML11_TEST_FIND_OR_FALLBACK(integer, local_date     );
    TOML11_TEST_FIND_OR_FALLBACK(integer, local_datetime );
    TOML11_TEST_FIND_OR_FALLBACK(integer, offset_datetime);
    TOML11_TEST_FIND_OR_FALLBACK(integer, array          );
    TOML11_TEST_FIND_OR_FALLBACK(integer, table          );

    TOML11_TEST_FIND_OR_FALLBACK(floating, boolean        );
    TOML11_TEST_FIND_OR_FALLBACK(floating, integer        );
    TOML11_TEST_FIND_OR_FALLBACK(floating, string         );
    TOML11_TEST_FIND_OR_FALLBACK(floating, local_time     );
    TOML11_TEST_FIND_OR_FALLBACK(floating, local_date     );
    TOML11_TEST_FIND_OR_FALLBACK(floating, local_datetime );
    TOML11_TEST_FIND_OR_FALLBACK(floating, offset_datetime);
    TOML11_TEST_FIND_OR_FALLBACK(floating, array          );
    TOML11_TEST_FIND_OR_FALLBACK(floating, table          );

    TOML11_TEST_FIND_OR_FALLBACK(string, boolean        );
    TOML11_TEST_FIND_OR_FALLBACK(string, integer        );
    TOML11_TEST_FIND_OR_FALLBACK(string, floating       );
    TOML11_TEST_FIND_OR_FALLBACK(string, local_time     );
    TOML11_TEST_FIND_OR_FALLBACK(string, local_date     );
    TOML11_TEST_FIND_OR_FALLBACK(string, local_datetime );
    TOML11_TEST_FIND_OR_FALLBACK(string, offset_datetime);
    TOML11_TEST_FIND_OR_FALLBACK(string, array          );
    TOML11_TEST_FIND_OR_FALLBACK(string, table          );

    TOML11_TEST_FIND_OR_FALLBACK(local_time, boolean        );
    TOML11_TEST_FIND_OR_FALLBACK(local_time, integer        );
    TOML11_TEST_FIND_OR_FALLBACK(local_time, floating       );
    TOML11_TEST_FIND_OR_FALLBACK(local_time, string         );
    TOML11_TEST_FIND_OR_FALLBACK(local_time, local_date     );
    TOML11_TEST_FIND_OR_FALLBACK(local_time, local_datetime );
    TOML11_TEST_FIND_OR_FALLBACK(local_time, offset_datetime);
    TOML11_TEST_FIND_OR_FALLBACK(local_time, array          );
    TOML11_TEST_FIND_OR_FALLBACK(local_time, table          );

    TOML11_TEST_FIND_OR_FALLBACK(local_date, boolean        );
    TOML11_TEST_FIND_OR_FALLBACK(local_date, integer        );
    TOML11_TEST_FIND_OR_FALLBACK(local_date, floating       );
    TOML11_TEST_FIND_OR_FALLBACK(local_date, string         );
    TOML11_TEST_FIND_OR_FALLBACK(local_date, local_time     );
    TOML11_TEST_FIND_OR_FALLBACK(local_date, local_datetime );
    TOML11_TEST_FIND_OR_FALLBACK(local_date, offset_datetime);
    TOML11_TEST_FIND_OR_FALLBACK(local_date, array          );
    TOML11_TEST_FIND_OR_FALLBACK(local_date, table          );

    TOML11_TEST_FIND_OR_FALLBACK(local_datetime, boolean        );
    TOML11_TEST_FIND_OR_FALLBACK(local_datetime, integer        );
    TOML11_TEST_FIND_OR_FALLBACK(local_datetime, floating       );
    TOML11_TEST_FIND_OR_FALLBACK(local_datetime, string         );
    TOML11_TEST_FIND_OR_FALLBACK(local_datetime, local_time     );
    TOML11_TEST_FIND_OR_FALLBACK(local_datetime, local_date     );
    TOML11_TEST_FIND_OR_FALLBACK(local_datetime, offset_datetime);
    TOML11_TEST_FIND_OR_FALLBACK(local_datetime, array          );
    TOML11_TEST_FIND_OR_FALLBACK(local_datetime, table          );

    TOML11_TEST_FIND_OR_FALLBACK(offset_datetime, boolean        );
    TOML11_TEST_FIND_OR_FALLBACK(offset_datetime, integer        );
    TOML11_TEST_FIND_OR_FALLBACK(offset_datetime, floating       );
    TOML11_TEST_FIND_OR_FALLBACK(offset_datetime, string         );
    TOML11_TEST_FIND_OR_FALLBACK(offset_datetime, local_time     );
    TOML11_TEST_FIND_OR_FALLBACK(offset_datetime, local_date     );
    TOML11_TEST_FIND_OR_FALLBACK(offset_datetime, local_datetime );
    TOML11_TEST_FIND_OR_FALLBACK(offset_datetime, array          );
    TOML11_TEST_FIND_OR_FALLBACK(offset_datetime, table          );

    TOML11_TEST_FIND_OR_FALLBACK(array, boolean        );
    TOML11_TEST_FIND_OR_FALLBACK(array, integer        );
    TOML11_TEST_FIND_OR_FALLBACK(array, floating       );
    TOML11_TEST_FIND_OR_FALLBACK(array, string         );
    TOML11_TEST_FIND_OR_FALLBACK(array, local_time     );
    TOML11_TEST_FIND_OR_FALLBACK(array, local_date     );
    TOML11_TEST_FIND_OR_FALLBACK(array, local_datetime );
    TOML11_TEST_FIND_OR_FALLBACK(array, offset_datetime);
    TOML11_TEST_FIND_OR_FALLBACK(array, table          );

    TOML11_TEST_FIND_OR_FALLBACK(table, boolean        );
    TOML11_TEST_FIND_OR_FALLBACK(table, integer        );
    TOML11_TEST_FIND_OR_FALLBACK(table, floating       );
    TOML11_TEST_FIND_OR_FALLBACK(table, string         );
    TOML11_TEST_FIND_OR_FALLBACK(table, local_time     );
    TOML11_TEST_FIND_OR_FALLBACK(table, local_date     );
    TOML11_TEST_FIND_OR_FALLBACK(table, local_datetime );
    TOML11_TEST_FIND_OR_FALLBACK(table, offset_datetime);
    TOML11_TEST_FIND_OR_FALLBACK(table, array          );
}
#undef TOML11_TEST_FIND_OR_FALLBACK

TEST_CASE("testing find_or; converting to an integer")
{
    {
        toml::value v = toml::table{{"num", 42}};
        CHECK_EQ(42u, toml::find_or(v, "num", 0u));
        CHECK_EQ(0u , toml::find_or(v, "foo", 0u));
    }
    {
        toml::value v = toml::table{{"num", 42}};
        const auto moved = toml::find_or(std::move(v), "num", 0u);
        CHECK_EQ(42u, moved);
    }
    {
        toml::value v = toml::table{{"num", 42}};
        const auto moved = toml::find_or(std::move(v), "foo", 0u);
        CHECK_EQ(0u,  moved);
    }
}

TEST_CASE("testing find_or; converting to a floating")
{
    {
        toml::value v1 = toml::table{{"key", 42}};
        toml::value v2 = toml::table{{"key", 3.14}};
        CHECK_EQ(2.71f, toml::find_or(v1, "key", 2.71f));
        const double ref(3.14);
        CHECK_EQ(static_cast<float>(ref), toml::find_or(v2, "key", 2.71f));
    }
    {
        toml::value v1 = toml::table{{"key", 42}};
        toml::value v2 = toml::table{{"key", 3.14}};
        const auto moved1 = toml::find_or(std::move(v1), "key", 2.71f);
        const auto moved2 = toml::find_or(std::move(v2), "key", 2.71f);
        CHECK_EQ(2.71f, moved1);
        const double ref(3.14);
        CHECK_EQ(static_cast<float>(ref), moved2);
    }
}

TEST_CASE("testing find_or; converting to a string")
{
    {
        toml::value v1 = toml::table{{"key", "foobar"}};
        toml::value v2 = toml::table{{"key", 42}};

        std::string       s1("bazqux");
        const std::string s2("bazqux");

        CHECK_EQ("foobar", toml::find_or(v1, "key", s1));
        CHECK_EQ("bazqux", toml::find_or(v2, "key", s1));

        std::string& v1r = toml::find_or(v1, "key", s1);
        std::string& s1r = toml::find_or(v2, "key", s1);

        CHECK_EQ("foobar", v1r);
        CHECK_EQ("bazqux", s1r);

        CHECK_EQ("foobar", toml::find_or(v1, "key", s2));
        CHECK_EQ("bazqux", toml::find_or(v2, "key", s2));

        CHECK_EQ("foobar", toml::find_or(std::move(v1), "key", std::move(s1)));
        s1 = "bazqux"; // restoring moved value
        CHECK_EQ("bazqux", toml::find_or(std::move(v2), "key", std::move(s1)));
    }
    {
        toml::value v1 = toml::table{{"key", "foobar"}};
        toml::value v2 = toml::table{{"key", 42}};

        std::string s1("bazqux");

        const auto moved1 = toml::find_or(std::move(v1), "key", s1);
        const auto moved2 = toml::find_or(std::move(v2), "key", s1);

        CHECK_EQ("foobar", moved1);
        CHECK_EQ("bazqux", moved2);
    }
    {
        toml::value v1 = toml::table{{"key", "foobar"}};
        toml::value v2 = toml::table{{"key", 42}};

        std::string s1("bazqux");
        std::string s2("bazqux");

        const auto moved1 = toml::find_or(std::move(v1), "key", std::move(s1));
        const auto moved2 = toml::find_or(std::move(v2), "key", std::move(s2));

        CHECK_EQ("foobar", moved1);
        CHECK_EQ("bazqux", moved2);
    }

    // string literal
    {
        toml::value v1 = toml::table{{"key", "foobar"}};
        toml::value v2 = toml::table{{"key",42}};

        CHECK_EQ("foobar", toml::find_or(v1, "key", "bazqux"));
        CHECK_EQ("bazqux", toml::find_or(v2, "key", "bazqux"));

        const auto lit = "bazqux";
        CHECK_EQ("foobar", toml::find_or(v1, "key", lit));
        CHECK_EQ("bazqux", toml::find_or(v2, "key", lit));
    }
    {
        toml::value v1 = toml::table{{"key", "foobar"}};
        toml::value v2 = toml::table{{"key",42}};

        const auto moved1 = toml::find_or(std::move(v1), "key", "bazqux");
        const auto moved2 = toml::find_or(std::move(v2), "key", "bazqux");

        CHECK_EQ("foobar", moved1);
        CHECK_EQ("bazqux", moved2);
    }
    {
        toml::value v1 = toml::table{{"key", "foobar"}};
        toml::value v2 = toml::table{{"key",42}};

        const char* lit = "bazqux";
        const auto moved1 = toml::find_or(std::move(v1), "key", lit);
        const auto moved2 = toml::find_or(std::move(v2), "key", lit);

        CHECK_EQ("foobar", moved1);
        CHECK_EQ("bazqux", moved2);
    }
}

TEST_CASE("testing find_or; converting to a map")
{
    using map_type = std::map<std::string, std::string>;
    {
        const toml::value v1 = toml::table{
            {"key", toml::table{{"key", "value"}}}
        };

        const auto key  = toml::find_or(v1, "key",  map_type{});
        const auto key2 = toml::find_or(v1, "key2", map_type{});

        CHECK_UNARY(!key.empty());
        CHECK_UNARY(key2.empty());

        CHECK_EQ(key.size()   , 1u);
        CHECK_EQ(key.at("key"), "value");
    }
    {
        toml::value v1 = toml::table{
            {"key", toml::table{{"key", "value"}}}
        };

        const auto key  = toml::find_or<map_type>(v1, "key",  map_type{});
        const auto key2 = toml::find_or<map_type>(v1, "key2", map_type{});

        CHECK_UNARY(!key.empty());
        CHECK_UNARY(key2.empty());

        CHECK_EQ(key.size()   , 1u);
        CHECK_EQ(key.at("key"), "value");
    }

    {
        toml::value v1 = toml::table{
            {"key", toml::table{{"key", "value"}}}
        };
        toml::value v2(v1);

        const auto key  = toml::find_or(std::move(v1), "key",  map_type{});
        const auto key2 = toml::find_or(std::move(v2), "key2", map_type{});

        CHECK_UNARY(!key.empty());
        CHECK_UNARY(key2.empty());

        CHECK_EQ(key.size()   , 1u);
        CHECK_EQ(key.at("key"), "value");
    }
    {
        toml::value v1 = toml::table{
            {"key", toml::table{{"key", "value"}}}
        };
        toml::value v2(v1);

        const auto key  = toml::find_or<map_type>(std::move(v1), "key",  map_type{});
        const auto key2 = toml::find_or<map_type>(std::move(v2), "key2", map_type{});

        CHECK_UNARY(!key.empty());
        CHECK_UNARY(key2.empty());

        CHECK_EQ(key.size()   , 1u);
        CHECK_EQ(key.at("key"), "value");
    }
}

TEST_CASE("testing find_or(val, keys..., opt)")
{
    // &, using type deduction
    {
        toml::value v(
            toml::table{ {"foo",
                toml::table{ {"bar",
                    toml::table{ {"baz",
                        "qux"
                    } }
                } }
            } }
        );
        std::string opt("hoge");

        auto& v1 = toml::find_or(v, "foo", "bar", "baz", opt);
        auto& v2 = toml::find_or(v, "foo", "bar", "qux", opt);

        CHECK_EQ(v1, "qux");
        CHECK_EQ(v2, "hoge");

        // v1 and v2 are mutable
        v1 = "hoge";
        v2 = "fuga";

        CHECK_EQ(v1, "hoge");
        CHECK_EQ(v2, "fuga");
    }
    // the same with deeper table
    {
        toml::value v(
            toml::table{ {"A",
            toml::table{ {"B",
            toml::table{ {"C",
            toml::table{ {"D",
            toml::table{ {"E",
            toml::table{ {"F",
                "foo"
            } }
            } }
            } }
            } }
            } }
            } }
        );
        std::string opt("bar");

        auto& v1 = toml::find_or(v, "A", "B", "C", "D", "E", "F", opt);
        auto& v2 = toml::find_or(v, "A", "B", "C", "D", "E", "G", opt);

        CHECK_EQ(v1, "foo");
        CHECK_EQ(v2, "bar");

        // v1 and v2 are mutable
        v1 = "hoge";
        v2 = "fuga";

        CHECK_EQ(v1, "hoge");
        CHECK_EQ(v2, "fuga");
    }

    // const&, type deduction
    {
        const toml::value v(
            toml::table{ {"foo",
                toml::table{ {"bar",
                    toml::table{ {"baz",
                        "qux"
                    } }
                } }
            } }
        );
        std::string opt("hoge");

        const auto& v1 = toml::find_or(v, "foo", "bar", "baz", opt);
        const auto& v2 = toml::find_or(v, "foo", "bar", "qux", opt);

        CHECK_EQ(v1, "qux");
        CHECK_EQ(v2, "hoge");
    }
    {
        toml::value v(
            toml::table{ {"A",
            toml::table{ {"B",
            toml::table{ {"C",
            toml::table{ {"D",
            toml::table{ {"E",
            toml::table{ {"F",
                "foo"
            } }
            } }
            } }
            } }
            } }
            } }
        );
        std::string opt("bar");

        const auto& v1 = toml::find_or(v, "A", "B", "C", "D", "E", "F", opt);
        const auto& v2 = toml::find_or(v, "A", "B", "C", "D", "E", "G", opt);

        CHECK_EQ(v1, "foo");
        CHECK_EQ(v2, "bar");
    }

    // explicitly specify type, doing type conversion
    {
        const toml::value v(
            toml::table{ {"foo",
                toml::table{ {"bar",
                    toml::table{ {"baz",
                        42
                    } }
                } }
            } }
        );
        int opt = 6 * 9;

        auto v1 = toml::find_or<int>(v, "foo", "bar", "baz", opt);
        auto v2 = toml::find_or<int>(v, "foo", "bar", "qux", opt);

        CHECK_EQ(v1, 42);
        CHECK_EQ(v2, 6*9);
    }
    {
        toml::value v(
            toml::table{ {"A",
            toml::table{ {"B",
            toml::table{ {"C",
            toml::table{ {"D",
            toml::table{ {"E",
            toml::table{ {"F",
                42
            } }
            } }
            } }
            } }
            } }
            } }
        );
        int opt = 6 * 9;

        auto v1 = toml::find_or<int>(v, "A", "B", "C", "D", "E", "F", opt);
        auto v2 = toml::find_or<int>(v, "A", "B", "C", "D", "E", "G", opt);

        CHECK_EQ(v1, 42);
        CHECK_EQ(v2, 6*9);
    }

    // the value exists, but type is different from the expected.
    {
        const toml::value v(
            toml::table{ {"foo",
                toml::table{ {"bar",
                    toml::table{ {"baz",
                        42
                    } }
                } }
            } }
        );
        auto v1 = toml::find_or<std::string>(v, "foo", "bar", "baz", "hoge");
        auto v2 = toml::find_or<double     >(v, "foo", "bar", "baz", 3.14);

        CHECK_EQ(v1, "hoge");
        CHECK_EQ(v2, 3.14);
    }
    {
        const toml::value v(
            toml::table{ {"A",
            toml::table{ {"B",
            toml::table{ {"C",
            toml::table{ {"D",
            toml::table{ {"E",
            toml::table{ {"F",
                42
            } }
            } }
            } }
            } }
            } }
            } }
        );
        auto v1 = toml::find_or<std::string>(v, "A", "B", "C", "D", "E", "F", "bar");
        auto v2 = toml::find_or<double     >(v, "A", "B", "C", "D", "E", "F", 3.14);

        CHECK_EQ(v1, "bar");
        CHECK_EQ(v2, 3.14);
    }
}

TEST_CASE("testing find_or_default(val, keys...)")
{
    // explicitly specify type, doing type conversion
    {
        const toml::value v(
            toml::table{ {"foo",
                toml::table{ {"bar",
                    toml::table{ {"baz",
                        42
                    } }
                } }
            } }
        );
        auto v1 = toml::find_or_default<int>(v, "foo", "bar", "baz");
        auto v2 = toml::find_or_default<int>(v, "foo", "bar", "qux");

        CHECK_EQ(v1, 42);
        CHECK_EQ(v2, 0);
    }
    {
        toml::value v(
            toml::table{ {"A",
            toml::table{ {"B",
            toml::table{ {"C",
            toml::table{ {"D",
            toml::table{ {"E",
            toml::table{ {"F",
                42
            } }
            } }
            } }
            } }
            } }
            } }
        );
        auto v1 = toml::find_or_default<int>(v, "A", "B", "C", "D", "E", "F");
        auto v2 = toml::find_or_default<int>(v, "A", "B", "C", "D", "E", "G");

        CHECK_EQ(v1, 42);
        CHECK_EQ(v2, 0);
    }

    // the value exists, but type is different from the expected.
    {
        const toml::value v(
            toml::table{ {"foo",
                toml::table{ {"bar",
                    toml::table{ {"baz",
                        42
                    } }
                } }
            } }
        );
        auto v1 = toml::find_or_default<std::string>(v, "foo", "bar", "baz");
        auto v2 = toml::find_or_default<double     >(v, "foo", "bar", "baz");

        CHECK_EQ(v1, "");
        CHECK_EQ(v2, 0.);
    }
    {
        const toml::value v(
            toml::table{ {"A",
            toml::table{ {"B",
            toml::table{ {"C",
            toml::table{ {"D",
            toml::table{ {"E",
            toml::table{ {"F",
                42
            } }
            } }
            } }
            } }
            } }
            } }
        );
        auto v1 = toml::find_or_default<std::string>(v, "A", "B", "C", "D", "E", "F");
        auto v2 = toml::find_or_default<double     >(v, "A", "B", "C", "D", "E", "F");

        CHECK_EQ(v1, "");
        CHECK_EQ(v2, 0.);
    }
}

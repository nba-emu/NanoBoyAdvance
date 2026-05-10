#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/value.hpp>

#include <deque>
#include <map>

template<typename TC>
void test_is_type(const toml::basic_value<TC>& v, const toml::value_t t)
{
    CHECK_EQ(v.type(), t);

    if(t == toml::value_t::boolean        ) {CHECK_UNARY(v.is_boolean()        );} else {CHECK_UNARY_FALSE(v.is_boolean()        );}
    if(t == toml::value_t::integer        ) {CHECK_UNARY(v.is_integer()        );} else {CHECK_UNARY_FALSE(v.is_integer()        );}
    if(t == toml::value_t::floating       ) {CHECK_UNARY(v.is_floating()       );} else {CHECK_UNARY_FALSE(v.is_floating()       );}
    if(t == toml::value_t::string         ) {CHECK_UNARY(v.is_string()         );} else {CHECK_UNARY_FALSE(v.is_string()         );}
    if(t == toml::value_t::offset_datetime) {CHECK_UNARY(v.is_offset_datetime());} else {CHECK_UNARY_FALSE(v.is_offset_datetime());}
    if(t == toml::value_t::local_datetime ) {CHECK_UNARY(v.is_local_datetime() );} else {CHECK_UNARY_FALSE(v.is_local_datetime() );}
    if(t == toml::value_t::local_date     ) {CHECK_UNARY(v.is_local_date()     );} else {CHECK_UNARY_FALSE(v.is_local_date()     );}
    if(t == toml::value_t::local_time     ) {CHECK_UNARY(v.is_local_time()     );} else {CHECK_UNARY_FALSE(v.is_local_time()     );}
    if(t == toml::value_t::array          ) {CHECK_UNARY(v.is_array()          );} else {CHECK_UNARY_FALSE(v.is_array()          );}
    if(t == toml::value_t::table          ) {CHECK_UNARY(v.is_table()          );} else {CHECK_UNARY_FALSE(v.is_table()          );}
    if(t == toml::value_t::empty          ) {CHECK_UNARY(v.is_empty()          );} else {CHECK_UNARY_FALSE(v.is_empty()          );}

    using value_type = toml::basic_value<TC>;
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

    if(t == toml::value_t::boolean        ) {CHECK_UNARY(v.template is<boolean_type>()        );} else {CHECK_UNARY_FALSE(v.template is<boolean_type>()        );}
    if(t == toml::value_t::integer        ) {CHECK_UNARY(v.template is<integer_type>()        );} else {CHECK_UNARY_FALSE(v.template is<integer_type>()        );}
    if(t == toml::value_t::floating       ) {CHECK_UNARY(v.template is<floating_type>()       );} else {CHECK_UNARY_FALSE(v.template is<floating_type>()       );}
    if(t == toml::value_t::string         ) {CHECK_UNARY(v.template is<string_type>()         );} else {CHECK_UNARY_FALSE(v.template is<string_type>()         );}
    if(t == toml::value_t::offset_datetime) {CHECK_UNARY(v.template is<offset_datetime_type>());} else {CHECK_UNARY_FALSE(v.template is<offset_datetime_type>());}
    if(t == toml::value_t::local_datetime ) {CHECK_UNARY(v.template is<local_datetime_type>() );} else {CHECK_UNARY_FALSE(v.template is<local_datetime_type>() );}
    if(t == toml::value_t::local_date     ) {CHECK_UNARY(v.template is<local_date_type>()     );} else {CHECK_UNARY_FALSE(v.template is<local_date_type>()     );}
    if(t == toml::value_t::local_time     ) {CHECK_UNARY(v.template is<local_time_type>()     );} else {CHECK_UNARY_FALSE(v.template is<local_time_type>()     );}
    if(t == toml::value_t::array          ) {CHECK_UNARY(v.template is<array_type>()          );} else {CHECK_UNARY_FALSE(v.template is<array_type>()          );}
    if(t == toml::value_t::table          ) {CHECK_UNARY(v.template is<table_type>()          );} else {CHECK_UNARY_FALSE(v.template is<table_type>()          );}
//     if(t == toml::value_t::empty          ) {CHECK_UNARY(v.template is</*no such type!*/>());} else {CHECK_UNARY_FALSE(v.template is</*no such type!*/>()   );}

    return;
}

template<typename TC>
void test_as_type_throws(toml::basic_value<TC>& v, const toml::value_t t)
{
    if(t != toml::value_t::boolean        ) {CHECK_THROWS_AS(v.as_boolean()        , toml::type_error); CHECK_THROWS_AS(v.template as<toml::value_t::boolean>()        , toml::type_error);}
    if(t != toml::value_t::integer        ) {CHECK_THROWS_AS(v.as_integer()        , toml::type_error); CHECK_THROWS_AS(v.template as<toml::value_t::integer>()        , toml::type_error);}
    if(t != toml::value_t::floating       ) {CHECK_THROWS_AS(v.as_floating()       , toml::type_error); CHECK_THROWS_AS(v.template as<toml::value_t::floating>()       , toml::type_error);}
    if(t != toml::value_t::string         ) {CHECK_THROWS_AS(v.as_string()         , toml::type_error); CHECK_THROWS_AS(v.template as<toml::value_t::string>()         , toml::type_error);}
    if(t != toml::value_t::offset_datetime) {CHECK_THROWS_AS(v.as_offset_datetime(), toml::type_error); CHECK_THROWS_AS(v.template as<toml::value_t::offset_datetime>(), toml::type_error);}
    if(t != toml::value_t::local_datetime ) {CHECK_THROWS_AS(v.as_local_datetime() , toml::type_error); CHECK_THROWS_AS(v.template as<toml::value_t::local_datetime>() , toml::type_error);}
    if(t != toml::value_t::local_date     ) {CHECK_THROWS_AS(v.as_local_date()     , toml::type_error); CHECK_THROWS_AS(v.template as<toml::value_t::local_date>()     , toml::type_error);}
    if(t != toml::value_t::local_time     ) {CHECK_THROWS_AS(v.as_local_time()     , toml::type_error); CHECK_THROWS_AS(v.template as<toml::value_t::local_time>()     , toml::type_error);}
    if(t != toml::value_t::array          ) {CHECK_THROWS_AS(v.as_array()          , toml::type_error); CHECK_THROWS_AS(v.template as<toml::value_t::array>()          , toml::type_error);}
    if(t != toml::value_t::table          ) {CHECK_THROWS_AS(v.as_table()          , toml::type_error); CHECK_THROWS_AS(v.template as<toml::value_t::table>()          , toml::type_error);}

    if(t != toml::value_t::boolean        ) {CHECK_THROWS_AS(as_const(v).as_boolean()        , toml::type_error); CHECK_THROWS_AS(as_const(v).template as<toml::value_t::boolean>()        , toml::type_error);}
    if(t != toml::value_t::integer        ) {CHECK_THROWS_AS(as_const(v).as_integer()        , toml::type_error); CHECK_THROWS_AS(as_const(v).template as<toml::value_t::integer>()        , toml::type_error);}
    if(t != toml::value_t::floating       ) {CHECK_THROWS_AS(as_const(v).as_floating()       , toml::type_error); CHECK_THROWS_AS(as_const(v).template as<toml::value_t::floating>()       , toml::type_error);}
    if(t != toml::value_t::string         ) {CHECK_THROWS_AS(as_const(v).as_string()         , toml::type_error); CHECK_THROWS_AS(as_const(v).template as<toml::value_t::string>()         , toml::type_error);}
    if(t != toml::value_t::offset_datetime) {CHECK_THROWS_AS(as_const(v).as_offset_datetime(), toml::type_error); CHECK_THROWS_AS(as_const(v).template as<toml::value_t::offset_datetime>(), toml::type_error);}
    if(t != toml::value_t::local_datetime ) {CHECK_THROWS_AS(as_const(v).as_local_datetime() , toml::type_error); CHECK_THROWS_AS(as_const(v).template as<toml::value_t::local_datetime>() , toml::type_error);}
    if(t != toml::value_t::local_date     ) {CHECK_THROWS_AS(as_const(v).as_local_date()     , toml::type_error); CHECK_THROWS_AS(as_const(v).template as<toml::value_t::local_date>()     , toml::type_error);}
    if(t != toml::value_t::local_time     ) {CHECK_THROWS_AS(as_const(v).as_local_time()     , toml::type_error); CHECK_THROWS_AS(as_const(v).template as<toml::value_t::local_time>()     , toml::type_error);}
    if(t != toml::value_t::array          ) {CHECK_THROWS_AS(as_const(v).as_array()          , toml::type_error); CHECK_THROWS_AS(as_const(v).template as<toml::value_t::array>()          , toml::type_error);}
    if(t != toml::value_t::table          ) {CHECK_THROWS_AS(as_const(v).as_table()          , toml::type_error); CHECK_THROWS_AS(as_const(v).template as<toml::value_t::table>()          , toml::type_error);}

    return;
}

template<toml::value_t VT>
struct as_type_tester;
#ifndef TOML11_TEST_DEFINE_AS_TYPE_TESTER
#define TOML11_TEST_DEFINE_AS_TYPE_TESTER(ty)                                \
    template<>                                                               \
    struct as_type_tester<toml::value_t::ty>                                 \
    {                                                                        \
        template<typename TC, typename T>                                    \
        static void test(toml::basic_value<TC>& v, const T& eq, const T& ne) \
        {                                                                    \
            CHECK_EQ(v.as_ ## ty(), eq);                                     \
            CHECK_NE(v.as_ ## ty(), ne);                                     \
            CHECK_EQ(as_const(v).as_ ## ty(), eq);                           \
            CHECK_NE(as_const(v).as_ ## ty(), ne);                           \
            if(v.type() == toml::value_t::ty)                                \
            {                                                                \
                CHECK_EQ(v.as_ ## ty(std::nothrow), eq);                     \
                CHECK_NE(v.as_ ## ty(std::nothrow), ne);                     \
                CHECK_EQ(as_const(v).as_ ## ty(std::nothrow), eq);           \
                CHECK_NE(as_const(v).as_ ## ty(std::nothrow), ne);           \
            }                                                                \
        }                                                                    \
    }; /**/
#endif // TOML11_TEST_DEFINE_AS_TYPE_TESTER
TOML11_TEST_DEFINE_AS_TYPE_TESTER(boolean        )
TOML11_TEST_DEFINE_AS_TYPE_TESTER(integer        )
TOML11_TEST_DEFINE_AS_TYPE_TESTER(floating       )
TOML11_TEST_DEFINE_AS_TYPE_TESTER(string         )
TOML11_TEST_DEFINE_AS_TYPE_TESTER(offset_datetime)
TOML11_TEST_DEFINE_AS_TYPE_TESTER(local_datetime )
TOML11_TEST_DEFINE_AS_TYPE_TESTER(local_date     )
TOML11_TEST_DEFINE_AS_TYPE_TESTER(local_time     )
TOML11_TEST_DEFINE_AS_TYPE_TESTER(array          )
TOML11_TEST_DEFINE_AS_TYPE_TESTER(table          )
#undef TOML11_TEST_DEFINE_AS_TYPE_TESTER

template<toml::value_t VT, typename TC, typename T, typename U>
void test_as_type(toml::basic_value<TC>& v, const T& eq_, const U& ne_)
{
    using elem_type = toml::detail::enum_to_type_t<VT, toml::basic_value<TC>>;
    const elem_type eq(eq_);
    const elem_type ne(ne_);

    as_type_tester<VT>::test(v, eq, ne);

    // as<T>
    CHECK_EQ(v.template as<VT>(), eq);
    CHECK_NE(v.template as<VT>(), ne);

    CHECK_EQ(as_const(v).template as<VT>(), eq);
    CHECK_NE(as_const(v).template as<VT>(), ne);

    if(v.type() == VT)
    {
        CHECK_EQ(v.template as<VT>(std::nothrow), eq);
        CHECK_NE(v.template as<VT>(std::nothrow), ne);

        CHECK_EQ(as_const(v).template as<VT>(std::nothrow), eq);
        CHECK_NE(as_const(v).template as<VT>(std::nothrow), ne);
    }
    return;
}

template<typename TC>
void test_as_type_fmt_throws(toml::basic_value<TC>& v, const toml::value_t t)
{
    if(t != toml::value_t::boolean        ) {CHECK_THROWS_AS(v.as_boolean_fmt()        , toml::type_error); CHECK_THROWS_AS(v.template as_fmt<toml::value_t::boolean>()        , toml::type_error);}
    if(t != toml::value_t::integer        ) {CHECK_THROWS_AS(v.as_integer_fmt()        , toml::type_error); CHECK_THROWS_AS(v.template as_fmt<toml::value_t::integer>()        , toml::type_error);}
    if(t != toml::value_t::floating       ) {CHECK_THROWS_AS(v.as_floating_fmt()       , toml::type_error); CHECK_THROWS_AS(v.template as_fmt<toml::value_t::floating>()       , toml::type_error);}
    if(t != toml::value_t::string         ) {CHECK_THROWS_AS(v.as_string_fmt()         , toml::type_error); CHECK_THROWS_AS(v.template as_fmt<toml::value_t::string>()         , toml::type_error);}
    if(t != toml::value_t::offset_datetime) {CHECK_THROWS_AS(v.as_offset_datetime_fmt(), toml::type_error); CHECK_THROWS_AS(v.template as_fmt<toml::value_t::offset_datetime>(), toml::type_error);}
    if(t != toml::value_t::local_datetime ) {CHECK_THROWS_AS(v.as_local_datetime_fmt() , toml::type_error); CHECK_THROWS_AS(v.template as_fmt<toml::value_t::local_datetime>() , toml::type_error);}
    if(t != toml::value_t::local_date     ) {CHECK_THROWS_AS(v.as_local_date_fmt()     , toml::type_error); CHECK_THROWS_AS(v.template as_fmt<toml::value_t::local_date>()     , toml::type_error);}
    if(t != toml::value_t::local_time     ) {CHECK_THROWS_AS(v.as_local_time_fmt()     , toml::type_error); CHECK_THROWS_AS(v.template as_fmt<toml::value_t::local_time>()     , toml::type_error);}
    if(t != toml::value_t::array          ) {CHECK_THROWS_AS(v.as_array_fmt()          , toml::type_error); CHECK_THROWS_AS(v.template as_fmt<toml::value_t::array>()          , toml::type_error);}
    if(t != toml::value_t::table          ) {CHECK_THROWS_AS(v.as_table_fmt()          , toml::type_error); CHECK_THROWS_AS(v.template as_fmt<toml::value_t::table>()          , toml::type_error);}

    if(t != toml::value_t::boolean        ) {CHECK_THROWS_AS(as_const(v).as_boolean_fmt()        , toml::type_error); CHECK_THROWS_AS(as_const(v).template as_fmt<toml::value_t::boolean>()        , toml::type_error);}
    if(t != toml::value_t::integer        ) {CHECK_THROWS_AS(as_const(v).as_integer_fmt()        , toml::type_error); CHECK_THROWS_AS(as_const(v).template as_fmt<toml::value_t::integer>()        , toml::type_error);}
    if(t != toml::value_t::floating       ) {CHECK_THROWS_AS(as_const(v).as_floating_fmt()       , toml::type_error); CHECK_THROWS_AS(as_const(v).template as_fmt<toml::value_t::floating>()       , toml::type_error);}
    if(t != toml::value_t::string         ) {CHECK_THROWS_AS(as_const(v).as_string_fmt()         , toml::type_error); CHECK_THROWS_AS(as_const(v).template as_fmt<toml::value_t::string>()         , toml::type_error);}
    if(t != toml::value_t::offset_datetime) {CHECK_THROWS_AS(as_const(v).as_offset_datetime_fmt(), toml::type_error); CHECK_THROWS_AS(as_const(v).template as_fmt<toml::value_t::offset_datetime>(), toml::type_error);}
    if(t != toml::value_t::local_datetime ) {CHECK_THROWS_AS(as_const(v).as_local_datetime_fmt() , toml::type_error); CHECK_THROWS_AS(as_const(v).template as_fmt<toml::value_t::local_datetime>() , toml::type_error);}
    if(t != toml::value_t::local_date     ) {CHECK_THROWS_AS(as_const(v).as_local_date_fmt()     , toml::type_error); CHECK_THROWS_AS(as_const(v).template as_fmt<toml::value_t::local_date>()     , toml::type_error);}
    if(t != toml::value_t::local_time     ) {CHECK_THROWS_AS(as_const(v).as_local_time_fmt()     , toml::type_error); CHECK_THROWS_AS(as_const(v).template as_fmt<toml::value_t::local_time>()     , toml::type_error);}
    if(t != toml::value_t::array          ) {CHECK_THROWS_AS(as_const(v).as_array_fmt()          , toml::type_error); CHECK_THROWS_AS(as_const(v).template as_fmt<toml::value_t::array>()          , toml::type_error);}
    if(t != toml::value_t::table          ) {CHECK_THROWS_AS(as_const(v).as_table_fmt()          , toml::type_error); CHECK_THROWS_AS(as_const(v).template as_fmt<toml::value_t::table>()          , toml::type_error);}

    return;
}

template<toml::value_t VT>
struct as_type_fmt_tester;
#ifndef TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER
#define TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(ty)                           \
    template<>                                                              \
    struct as_type_fmt_tester<toml::value_t::ty>                            \
    {                                                                       \
        template<typename TC, typename FMT>                                 \
        static void test(toml::basic_value<TC>& v, const FMT& fmt)          \
        {                                                                   \
            CHECK_EQ(         v .as_ ## ty ## _fmt(), fmt);                 \
            CHECK_EQ(as_const(v).as_ ## ty ## _fmt(), fmt);                 \
            if(v.type() == toml::value_t::ty)                               \
            {                                                               \
                CHECK_EQ(         v .as_ ## ty ## _fmt(std::nothrow), fmt); \
                CHECK_EQ(as_const(v).as_ ## ty ## _fmt(std::nothrow), fmt); \
            }                                                               \
        }                                                                   \
    }; /**/
#endif // TOML11_TEST_DEFINE_AS_TYPE_TESTER
TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(boolean        )
TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(integer        )
TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(floating       )
TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(string         )
TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(offset_datetime)
TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(local_datetime )
TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(local_date     )
TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(local_time     )
TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(array          )
TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER(table          )
#undef TOML11_TEST_DEFINE_AS_TYPE_FMT_TESTER

template<toml::value_t VT, typename TC, typename FMT>
void test_as_type_fmt(toml::basic_value<TC>& v, const FMT& fmt)
{
    as_type_fmt_tester<VT>::test(v, fmt);

    // as<T>
    CHECK_EQ(v          .template as_fmt<VT>(), fmt);
    CHECK_EQ(as_const(v).template as_fmt<VT>(), fmt);
    if(v.type() == VT)
    {
        CHECK_EQ(v          .template as_fmt<VT>(std::nothrow), fmt);
        CHECK_EQ(as_const(v).template as_fmt<VT>(std::nothrow), fmt);
    }
    return;
}

template<toml::value_t TYPE, typename T, typename U, typename FMT>
void test_ctors(T eq, U ne, FMT fmt)
{
    {
        toml::value x(eq);

        test_is_type           (x, TYPE);
        test_as_type_throws    (x, TYPE);
        test_as_type_fmt_throws(x, TYPE);

        test_as_type<TYPE>(x, eq, ne);

        CHECK_EQ(x.comments().size(), 0);
        CHECK_EQ(x.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_with_comments(eq, std::vector<std::string>{"foo", "bar"});

        test_is_type           (x_with_comments, TYPE);
        test_as_type_throws    (x_with_comments, TYPE);
        test_as_type_fmt_throws(x_with_comments, TYPE);

        test_as_type<TYPE>(x_with_comments, eq, ne);

        CHECK_EQ(x_with_comments.comments().size(), 2);
        CHECK_EQ(x_with_comments.comments().at(0), "foo");
        CHECK_EQ(x_with_comments.comments().at(1), "bar");

        CHECK_EQ(x_with_comments.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_with_format(eq, fmt);

        test_is_type           (x_with_format, TYPE);
        test_as_type_throws    (x_with_format, TYPE);
        test_as_type_fmt_throws(x_with_format, TYPE);

        test_as_type    <TYPE>(x_with_format, eq, ne);
        test_as_type_fmt<TYPE>(x_with_format, fmt);

        CHECK_EQ(x_with_format.comments().size(), 0);
        CHECK_EQ(x_with_format.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_with_com_fmt(eq, fmt,
                std::vector<std::string>{"foo", "bar"});

        test_is_type           (x_with_com_fmt, TYPE);
        test_as_type_throws    (x_with_com_fmt, TYPE);
        test_as_type_fmt_throws(x_with_com_fmt, TYPE);

        test_as_type    <TYPE>(x_with_com_fmt, eq, ne);
        test_as_type_fmt<TYPE>(x_with_com_fmt, fmt);

        CHECK_EQ(x_with_com_fmt.comments().size(), 2);
        CHECK_EQ(x_with_com_fmt.comments().at(0), "foo");
        CHECK_EQ(x_with_com_fmt.comments().at(1), "bar");

        CHECK_EQ(x_with_com_fmt.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_assign(ne, fmt);
        x_assign = eq;

        test_is_type           (x_assign, TYPE);
        test_as_type_throws    (x_assign, TYPE);
        test_as_type_fmt_throws(x_assign, TYPE);

        test_as_type    <TYPE>(x_assign, eq, ne);
        test_as_type_fmt<TYPE>(x_assign, fmt);

        CHECK_EQ(x_assign.comments().size(), 0);
        CHECK_EQ(x_assign.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_assign_different_type(true);
        x_assign_different_type = eq;

        test_is_type           (x_assign_different_type, TYPE);
        test_as_type_throws    (x_assign_different_type, TYPE);
        test_as_type_fmt_throws(x_assign_different_type, TYPE);

        test_as_type<TYPE>(x_assign_different_type, eq, ne);

        CHECK_EQ(x_assign_different_type.comments().size(), 0);
        CHECK_EQ(x_assign_different_type.location().is_ok(), false);
    }
}
template<toml::value_t TYPE, typename T, typename U, typename FMT>
void test_copy_move_ctors(T eq, U ne, FMT fmt)
{
    {
        toml::value x_original(eq, fmt, std::vector<std::string>{"foo", "bar"});
        toml::value x_copy(x_original);

        test_is_type           (x_copy, TYPE);
        test_as_type_throws    (x_copy, TYPE);
        test_as_type_fmt_throws(x_copy, TYPE);

        test_as_type    <TYPE>(x_copy, eq, ne);
        test_as_type_fmt<TYPE>(x_copy, fmt);

        CHECK_EQ(x_copy.comments().size(), 2);
        CHECK_EQ(x_copy.comments().at(0), "foo");
        CHECK_EQ(x_copy.comments().at(1), "bar");

        CHECK_EQ(x_copy.location().is_ok(), false);

        x_copy = 0; // reset with integer 0
        x_copy = x_original;

        test_is_type           (x_copy, TYPE);
        test_as_type_throws    (x_copy, TYPE);
        test_as_type_fmt_throws(x_copy, TYPE);

        test_as_type    <TYPE>(x_copy, eq, ne);
        test_as_type_fmt<TYPE>(x_copy, fmt);

        CHECK_EQ(x_copy.comments().size(), 2);
        CHECK_EQ(x_copy.comments().at(0), "foo");
        CHECK_EQ(x_copy.comments().at(1), "bar");

        CHECK_EQ(x_copy.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_original(eq, fmt, std::vector<std::string>{"foo", "bar"});
        toml::value x_move(std::move(x_original));

        test_is_type           (x_move, TYPE);
        test_as_type_throws    (x_move, TYPE);
        test_as_type_fmt_throws(x_move, TYPE);

        test_as_type    <TYPE>(x_move, eq, ne);
        test_as_type_fmt<TYPE>(x_move, fmt);

        CHECK_EQ(x_move.comments().size(), 2);
        CHECK_EQ(x_move.comments().at(0), "foo");
        CHECK_EQ(x_move.comments().at(1), "bar");

        CHECK_EQ(x_move.location().is_ok(), false);

        x_move = 0; // reset with integer 0

        toml::value x_original2(eq, fmt, std::vector<std::string>{"foo", "bar"});
        x_move = std::move(x_original2);

        test_is_type           (x_move, TYPE);
        test_as_type_throws    (x_move, TYPE);
        test_as_type_fmt_throws(x_move, TYPE);

        test_as_type    <TYPE>(x_move, eq, ne);
        test_as_type_fmt<TYPE>(x_move, fmt);

        CHECK_EQ(x_move.comments().size(), 2);
        CHECK_EQ(x_move.comments().at(0), "foo");
        CHECK_EQ(x_move.comments().at(1), "bar");

        CHECK_EQ(x_move.location().is_ok(), false);
    }
}

TEST_CASE("testing default constructor (empty)")
{
    toml::value v;

    test_is_type(v, toml::value_t::empty);

    CHECK_EQ(v.comments().size(), 0);
    CHECK_EQ(v.location().is_ok(), false);

    toml::value v1(v);

    test_is_type(v1, toml::value_t::empty);

    CHECK_EQ(v1.comments().size(), 0);
    CHECK_EQ(v1.location().is_ok(), false);

    toml::value v2(std::move(v));

    test_is_type(v2, toml::value_t::empty);

    CHECK_EQ(v2.comments().size(), 0);
    CHECK_EQ(v2.location().is_ok(), false);

    toml::value v3(std::move(v), std::vector<std::string>{"foo", "bar"});

    test_is_type(v3, toml::value_t::empty);

    CHECK_EQ(v3.comments().size(), 2);
    CHECK_EQ(v3.comments().at(0), "foo");
    CHECK_EQ(v3.comments().at(1), "bar");
    CHECK_EQ(v3.location().is_ok(), false);
}

TEST_CASE("testing constructor (boolean)")
{
    toml::boolean_format_info fmt;

    const bool eq = true;
    const bool ne = false;

    test_ctors          <toml::value_t::boolean>(eq, ne, fmt);
    test_copy_move_ctors<toml::value_t::boolean>(eq, ne, fmt);
}

TEST_CASE("testing constructor (integer)")
{
    toml::integer_format_info fmt;
    fmt.width = 10;

    const int eq = 42;
    const int ne = 6 * 9;

    test_ctors          <toml::value_t::integer>(eq, ne, fmt);
    test_copy_move_ctors<toml::value_t::integer>(eq, ne, fmt);
}

TEST_CASE("testing constructor (floating)")
{
    toml::floating_format_info fmt;
    fmt.fmt   = toml::floating_format::fixed;
    fmt.prec  = 6;

    const double eq = 3.14;
    const double ne = 2.71;

    test_ctors          <toml::value_t::floating>(eq, ne, fmt);
    test_copy_move_ctors<toml::value_t::floating>(eq, ne, fmt);
}

TEST_CASE("testing constructor (string)")
{
    toml::string_format_info fmt;
    fmt.fmt    = toml::string_format::literal;

    const std::string eq("hoge");
    const std::string ne("fuga");

    test_ctors          <toml::value_t::string>(eq, ne, fmt);
    test_copy_move_ctors<toml::value_t::string>(eq, ne, fmt);
}

TEST_CASE("testing constructor (literal string)")
{
    toml::string_format_info fmt;
    fmt.fmt    = toml::string_format::literal;

    // to explicitly pass (const char&)[N], we write it here directly
    // {{{

    // -----------------------------------------------------------------------
    {
        toml::value x("foo");

        test_is_type(x, toml::value_t::string);
        test_as_type_throws(x, toml::value_t::string);
        test_as_type_fmt_throws(x, toml::value_t::string);

        test_as_type<toml::value_t::string>(x, "foo", "bar");

        CHECK_EQ(x.comments().size(), 0);

        CHECK_EQ(x.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_with_comments("foo", std::vector<std::string>{"foo", "bar"});

        test_is_type           (x_with_comments, toml::value_t::string);
        test_as_type_throws    (x_with_comments, toml::value_t::string);
        test_as_type_fmt_throws(x_with_comments, toml::value_t::string);

        test_as_type<toml::value_t::string>(x_with_comments, "foo", "bar");

        CHECK_EQ(x_with_comments.comments().size(), 2);
        CHECK_EQ(x_with_comments.comments().at(0), "foo");
        CHECK_EQ(x_with_comments.comments().at(1), "bar");

        CHECK_EQ(x_with_comments.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_with_format("foo", fmt);

        test_is_type           (x_with_format, toml::value_t::string);
        test_as_type_throws    (x_with_format, toml::value_t::string);
        test_as_type_fmt_throws(x_with_format, toml::value_t::string);

        test_as_type    <toml::value_t::string>(x_with_format, "foo", "bar");
        test_as_type_fmt<toml::value_t::string>(x_with_format, fmt);

        CHECK_EQ(x_with_format.comments().size(), 0);

        CHECK_EQ(x_with_format.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_with_com_fmt("foo", fmt,
                std::vector<std::string>{"foo", "bar"});

        test_is_type           (x_with_com_fmt, toml::value_t::string);
        test_as_type_throws    (x_with_com_fmt, toml::value_t::string);
        test_as_type_fmt_throws(x_with_com_fmt, toml::value_t::string);

        test_as_type    <toml::value_t::string>(x_with_com_fmt, "foo", "bar");
        test_as_type_fmt<toml::value_t::string>(x_with_com_fmt, fmt);

        CHECK_EQ(x_with_com_fmt.comments().size(), 2);
        CHECK_EQ(x_with_com_fmt.comments().at(0), "foo");
        CHECK_EQ(x_with_com_fmt.comments().at(1), "bar");

        CHECK_EQ(x_with_com_fmt.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_from_string(std::string("bar"), fmt);
        x_from_string = "foo";

        test_is_type           (x_from_string, toml::value_t::string);
        test_as_type_throws    (x_from_string, toml::value_t::string);
        test_as_type_fmt_throws(x_from_string, toml::value_t::string);

        test_as_type    <toml::value_t::string>(x_from_string, "foo", "bar");
        test_as_type_fmt<toml::value_t::string>(x_from_string, fmt);

        CHECK_EQ(x_from_string.comments().size(), 0);

        CHECK_EQ(x_from_string.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_from_non_string(true);
        x_from_non_string = "foo";

        test_is_type           (x_from_non_string, toml::value_t::string);
        test_as_type_throws    (x_from_non_string, toml::value_t::string);
        test_as_type_fmt_throws(x_from_non_string, toml::value_t::string);

        test_as_type<toml::value_t::string>(x_from_non_string, "foo", "bar");

        CHECK_EQ(x_from_non_string.comments().size(), 0);

        CHECK_EQ(x_from_non_string.location().is_ok(), false);
    }
    // }}}
}

#ifdef TOML11_HAS_STRING_VIEW
#include <string_view>

TEST_CASE("testing constructor (string_view)")
{
    toml::string_format_info fmt;
    fmt.fmt    = toml::string_format::literal;

    const std::string_view eq("hoge");
    const std::string_view ne("fuga");

    test_ctors<toml::value_t::string>(eq, ne, fmt);
}
#endif

#ifdef TOML11_HAS_CHAR8_T
TEST_CASE("testing constructor (u8string)")
{
    toml::string_format_info fmt;
    fmt.fmt    = toml::string_format::basic;

    const std::string eq("hoge");
    const std::string ne("fuga");

    const std::u8string ref(u8"hoge");

    {
        toml::value x(ref);
        test_is_type(x, toml::value_t::string);
        test_as_type_throws(x, toml::value_t::string);
        test_as_type_fmt_throws(x, toml::value_t::string);

        test_as_type<toml::value_t::string>(x, "hoge", "fuga");
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_with_comments(ref, std::vector<std::string>{"foo", "bar"});

        test_is_type           (x_with_comments, toml::value_t::string);
        test_as_type_throws    (x_with_comments, toml::value_t::string);
        test_as_type_fmt_throws(x_with_comments, toml::value_t::string);

        test_as_type<toml::value_t::string>(x_with_comments, eq, ne);

        CHECK_EQ(x_with_comments.comments().size(), 2);
        CHECK_EQ(x_with_comments.comments().at(0), "foo");
        CHECK_EQ(x_with_comments.comments().at(1), "bar");

        CHECK_EQ(x_with_comments.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_with_format(eq, fmt);

        test_is_type           (x_with_format, toml::value_t::string);
        test_as_type_throws    (x_with_format, toml::value_t::string);
        test_as_type_fmt_throws(x_with_format, toml::value_t::string);

        test_as_type    <toml::value_t::string>(x_with_format, eq, ne);
        test_as_type_fmt<toml::value_t::string>(x_with_format, fmt);

        CHECK_EQ(x_with_format.comments().size(), 0);
        CHECK_EQ(x_with_format.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_with_com_fmt(ref, fmt,
                std::vector<std::string>{"foo", "bar"});

        test_is_type           (x_with_com_fmt, toml::value_t::string);
        test_as_type_throws    (x_with_com_fmt, toml::value_t::string);
        test_as_type_fmt_throws(x_with_com_fmt, toml::value_t::string);

        test_as_type    <toml::value_t::string>(x_with_com_fmt, eq, ne);
        test_as_type_fmt<toml::value_t::string>(x_with_com_fmt, fmt);

        CHECK_EQ(x_with_com_fmt.comments().size(), 2);
        CHECK_EQ(x_with_com_fmt.comments().at(0), "foo");
        CHECK_EQ(x_with_com_fmt.comments().at(1), "bar");

        CHECK_EQ(x_with_com_fmt.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_assign(ne, fmt);
        x_assign = ref;

        test_is_type           (x_assign, toml::value_t::string);
        test_as_type_throws    (x_assign, toml::value_t::string);
        test_as_type_fmt_throws(x_assign, toml::value_t::string);

        test_as_type    <toml::value_t::string>(x_assign, eq, ne);
        test_as_type_fmt<toml::value_t::string>(x_assign, fmt);

        CHECK_EQ(x_assign.comments().size(), 0);
        CHECK_EQ(x_assign.location().is_ok(), false);
    }
    // -----------------------------------------------------------------------
    {
        toml::value x_assign_different_type(true);
        x_assign_different_type = ref;

        test_is_type           (x_assign_different_type, toml::value_t::string);
        test_as_type_throws    (x_assign_different_type, toml::value_t::string);
        test_as_type_fmt_throws(x_assign_different_type, toml::value_t::string);

        test_as_type<toml::value_t::string>(x_assign_different_type, eq, ne);

        CHECK_EQ(x_assign_different_type.comments().size(), 0);
        CHECK_EQ(x_assign_different_type.location().is_ok(), false);
    }
}
#endif

TEST_CASE("testing constructor (local_date)")
{
    toml::local_date_format_info fmt;

    const toml::local_date eq(2023, toml::month_t::Feb, 2);
    const toml::local_date ne(2023, toml::month_t::Jan, 1);

    test_ctors          <toml::value_t::local_date>(eq, ne, fmt);
    test_copy_move_ctors<toml::value_t::local_date>(eq, ne, fmt);
}

TEST_CASE("testing constructor (local_time)")
{
    toml::local_time_format_info fmt;
    fmt.subsecond_precision = 3;

    const toml::local_time eq(12, 30, 45);
    const toml::local_time ne( 2, 34, 56);

    test_ctors          <toml::value_t::local_time>(eq, ne, fmt);
    test_copy_move_ctors<toml::value_t::local_time>(eq, ne, fmt);
}

TEST_CASE("testing constructor (std::chrono::duration)")
{
    toml::local_time_format_info fmt;
    fmt.subsecond_precision = 3;

    const auto eq = std::chrono::hours(12)  ;
    const auto ne = std::chrono::seconds(12);

    test_ctors<toml::value_t::local_time>(eq, ne, fmt);
}

TEST_CASE("testing constructor (local_datetime)")
{
    toml::local_datetime_format_info fmt;
    fmt.subsecond_precision = 3;

    const toml::local_datetime eq(toml::local_date(2023, toml::month_t::Feb, 2),
                                  toml::local_time(12, 30, 45));
    const toml::local_datetime ne(toml::local_date(2023, toml::month_t::Jan, 1),
                                  toml::local_time( 2, 34, 56));

    test_ctors          <toml::value_t::local_datetime>(eq, ne, fmt);
    test_copy_move_ctors<toml::value_t::local_datetime>(eq, ne, fmt);
}

TEST_CASE("testing constructor (offset_datetime)")
{
    toml::offset_datetime_format_info fmt;
    fmt.subsecond_precision = 3;

    const toml::offset_datetime eq(toml::local_date(2023, toml::month_t::Feb, 2),
                                   toml::local_time(12, 30, 45),
                                   toml::time_offset(9, 0));
    const toml::offset_datetime ne(toml::local_date(2023, toml::month_t::Jan, 1),
                                   toml::local_time( 2, 34, 56),
                                   toml::time_offset(-9, 0));

    test_ctors          <toml::value_t::offset_datetime>(eq, ne, fmt);
    test_copy_move_ctors<toml::value_t::offset_datetime>(eq, ne, fmt);
}

TEST_CASE("testing constructor (system_clock::time_point)")
{
    toml::offset_datetime_format_info fmt;
    fmt.subsecond_precision = 3;

    // we use zero-offset because offset_datetime uses gmtime.
    const std::chrono::system_clock::time_point eq =
        toml::offset_datetime(toml::local_date(2023, toml::month_t::Feb, 2),
                              toml::local_time(12, 30, 45),
                              toml::time_offset(0, 0));
    const std::chrono::system_clock::time_point ne =
        toml::offset_datetime(toml::local_date(2023, toml::month_t::Jan, 1),
                              toml::local_time( 2, 34, 56),
                              toml::time_offset(-9, 0));

    test_ctors<toml::value_t::offset_datetime>(eq, ne, fmt);
}

TEST_CASE("testing constructor (array)")
{
    toml::array_format_info fmt;
    fmt.body_indent = 10;
    fmt.closing_indent = 5;

    const toml::array eq{true,  42,   "hoge"};
    const toml::array ne{false, 3.14, "fuga"};

    test_ctors          <toml::value_t::array>(eq, ne, fmt);
    test_copy_move_ctors<toml::value_t::array>(eq, ne, fmt);
}

TEST_CASE("testing constructor (array-like)")
{
    toml::array_format_info fmt;
    fmt.body_indent = 10;
    fmt.closing_indent = 5;

    toml::value x(std::deque<toml::value>{true, 42, "hoge"});

    test_is_type(x, toml::value_t::array);
    test_as_type_throws(x, toml::value_t::array);
    test_as_type_fmt_throws(x, toml::value_t::array);

    CHECK_UNARY(x.at(0).is_boolean());
    CHECK_UNARY(x.at(1).is_integer());
    CHECK_UNARY(x.at(2).is_string());

    CHECK_EQ(x.at(0).as_boolean(), true);
    CHECK_EQ(x.at(1).as_integer(), 42);
    CHECK_EQ(x.at(2).as_string(),  std::string("hoge"));

    // -----------------------------------------------------------------------

    toml::value x_with_comments(std::deque<toml::value>{true, 42, "hoge"}, std::vector<std::string>{"foo", "bar"});

    test_is_type           (x_with_comments, toml::value_t::array);
    test_as_type_throws    (x_with_comments, toml::value_t::array);
    test_as_type_fmt_throws(x_with_comments, toml::value_t::array);

    CHECK_UNARY(x_with_comments.at(0).is_boolean());
    CHECK_UNARY(x_with_comments.at(1).is_integer());
    CHECK_UNARY(x_with_comments.at(2).is_string());

    CHECK_EQ(x_with_comments.at(0).as_boolean(), true);
    CHECK_EQ(x_with_comments.at(1).as_integer(), 42);
    CHECK_EQ(x_with_comments.at(2).as_string(),  std::string("hoge"));

    CHECK_EQ(x_with_comments.comments().size(), 2);
    CHECK_EQ(x_with_comments.comments().at(0), "foo");
    CHECK_EQ(x_with_comments.comments().at(1), "bar");

    CHECK_EQ(x_with_comments.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_with_format(std::deque<toml::value>{true, 42, "hoge"}, fmt);

    test_is_type           (x_with_format, toml::value_t::array);
    test_as_type_throws    (x_with_format, toml::value_t::array);
    test_as_type_fmt_throws(x_with_format, toml::value_t::array);

    CHECK_UNARY(x_with_format.at(0).is_boolean());
    CHECK_UNARY(x_with_format.at(1).is_integer());
    CHECK_UNARY(x_with_format.at(2).is_string());

    CHECK_EQ(x_with_format.at(0).as_boolean(), true);
    CHECK_EQ(x_with_format.at(1).as_integer(), 42);
    CHECK_EQ(x_with_format.at(2).as_string(),  std::string("hoge"));

    CHECK_EQ(x_with_format.as_array_fmt(), fmt);

    CHECK_EQ(x_with_format.comments().size(), 0);

    CHECK_EQ(x_with_format.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_with_com_fmt(std::deque<toml::value>{true, 42, "hoge"}, fmt,
            std::vector<std::string>{"foo", "bar"});

    test_is_type           (x_with_com_fmt, toml::value_t::array);
    test_as_type_throws    (x_with_com_fmt, toml::value_t::array);
    test_as_type_fmt_throws(x_with_com_fmt, toml::value_t::array);

    CHECK_UNARY(x_with_com_fmt.at(0).is_boolean());
    CHECK_UNARY(x_with_com_fmt.at(1).is_integer());
    CHECK_UNARY(x_with_com_fmt.at(2).is_string());

    CHECK_EQ(x_with_com_fmt.at(0).as_boolean(), true);
    CHECK_EQ(x_with_com_fmt.at(1).as_integer(), 42);
    CHECK_EQ(x_with_com_fmt.at(2).as_string(),  std::string("hoge"));

    CHECK_EQ(x_with_com_fmt.as_array_fmt(), fmt);

    CHECK_EQ(x_with_com_fmt.comments().size(), 2);
    CHECK_EQ(x_with_com_fmt.comments().at(0), "foo");
    CHECK_EQ(x_with_com_fmt.comments().at(1), "bar");

    CHECK_EQ(x_with_com_fmt.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_from_array(std::deque<toml::value>{3.14, 2.71, "foo"}, fmt);
    x_from_array = std::deque<toml::value>{true, 42, "hoge"};

    test_is_type           (x_from_array, toml::value_t::array);
    test_as_type_throws    (x_from_array, toml::value_t::array);
    test_as_type_fmt_throws(x_from_array, toml::value_t::array);

    CHECK_UNARY(x_from_array.at(0).is_boolean());
    CHECK_UNARY(x_from_array.at(1).is_integer());
    CHECK_UNARY(x_from_array.at(2).is_string());

    CHECK_EQ(x_from_array.at(0).as_boolean(), true);
    CHECK_EQ(x_from_array.at(1).as_integer(), 42);
    CHECK_EQ(x_from_array.at(2).as_string(),  std::string("hoge"));

    CHECK_EQ(x_from_array.as_array_fmt(), fmt);

    CHECK_EQ(x_from_array.comments().size(), 0);

    CHECK_EQ(x_from_array.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_from_non_array(true);
    x_from_non_array = std::deque<toml::value>{true, 42, "hoge"};

    test_is_type           (x_from_non_array, toml::value_t::array);
    test_as_type_throws    (x_from_non_array, toml::value_t::array);
    test_as_type_fmt_throws(x_from_non_array, toml::value_t::array);

    CHECK_UNARY(x_from_non_array.at(0).is_boolean());
    CHECK_UNARY(x_from_non_array.at(1).is_integer());
    CHECK_UNARY(x_from_non_array.at(2).is_string());

    CHECK_EQ(x_from_non_array.at(0).as_boolean(), true);
    CHECK_EQ(x_from_non_array.at(1).as_integer(), 42);
    CHECK_EQ(x_from_non_array.at(2).as_string(),  std::string("hoge"));

    CHECK_EQ(x_from_non_array.comments().size(), 0);

    CHECK_EQ(x_from_non_array.location().is_ok(), false);
}

TEST_CASE("testing constructor (table)")
{
    toml::table_format_info fmt;
    fmt.body_indent = 10;

    const toml::table eq{{"a",     true }, {"b", 42}, {"c", "hoge"}};
    const toml::table ne{{"alpha", false}, {"b", 42}, {"c", "fuga"}};

    test_ctors          <toml::value_t::table>(eq, ne, fmt);
    test_copy_move_ctors<toml::value_t::table>(eq, ne, fmt);
}

TEST_CASE("testing constructor (table-like)")
{
    toml::table_format_info fmt;
    fmt.body_indent = 10;

    toml::value x(std::map<std::string, toml::value>{{"a", true}, {"b", 42}, {"c", "hoge"}});

    test_is_type(x, toml::value_t::table);
    test_as_type_throws(x, toml::value_t::table);
    test_as_type_fmt_throws(x, toml::value_t::table);

    CHECK_UNARY(x.at("a").is_boolean());
    CHECK_UNARY(x.at("b").is_integer());
    CHECK_UNARY(x.at("c").is_string());

    CHECK_EQ(x.at("a").as_boolean(), true);
    CHECK_EQ(x.at("b").as_integer(), 42);
    CHECK_EQ(x.at("c").as_string(),  std::string("hoge"));

    // -----------------------------------------------------------------------

    toml::value x_with_comments(std::map<std::string, toml::value>{{"a", true}, {"b", 42}, {"c", "hoge"}}, std::vector<std::string>{"foo", "bar"});

    test_is_type           (x_with_comments, toml::value_t::table);
    test_as_type_throws    (x_with_comments, toml::value_t::table);
    test_as_type_fmt_throws(x_with_comments, toml::value_t::table);

    CHECK_UNARY(x_with_comments.at("a").is_boolean());
    CHECK_UNARY(x_with_comments.at("b").is_integer());
    CHECK_UNARY(x_with_comments.at("c").is_string());

    CHECK_EQ(x_with_comments.at("a").as_boolean(), true);
    CHECK_EQ(x_with_comments.at("b").as_integer(), 42);
    CHECK_EQ(x_with_comments.at("c").as_string(),  std::string("hoge"));

    CHECK_EQ(x_with_comments.comments().size(), 2);
    CHECK_EQ(x_with_comments.comments().at(0), "foo");
    CHECK_EQ(x_with_comments.comments().at(1), "bar");

    CHECK_EQ(x_with_comments.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_with_format(std::map<std::string, toml::value>{{"a", true}, {"b", 42}, {"c", "hoge"}}, fmt);

    test_is_type           (x_with_format, toml::value_t::table);
    test_as_type_throws    (x_with_format, toml::value_t::table);
    test_as_type_fmt_throws(x_with_format, toml::value_t::table);

    CHECK_UNARY(x_with_format.at("a").is_boolean());
    CHECK_UNARY(x_with_format.at("b").is_integer());
    CHECK_UNARY(x_with_format.at("c").is_string());

    CHECK_EQ(x_with_format.at("a").as_boolean(), true);
    CHECK_EQ(x_with_format.at("b").as_integer(), 42);
    CHECK_EQ(x_with_format.at("c").as_string(),  std::string("hoge"));

    CHECK_EQ(x_with_format.as_table_fmt(), fmt);

    CHECK_EQ(x_with_format.comments().size(), 0);

    CHECK_EQ(x_with_format.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_with_com_fmt(std::map<std::string, toml::value>{{"a", true}, {"b", 42}, {"c", "hoge"}}, fmt,
            std::vector<std::string>{"foo", "bar"});

    test_is_type           (x_with_com_fmt, toml::value_t::table);
    test_as_type_throws    (x_with_com_fmt, toml::value_t::table);
    test_as_type_fmt_throws(x_with_com_fmt, toml::value_t::table);

    CHECK_UNARY(x_with_com_fmt.at("a").is_boolean());
    CHECK_UNARY(x_with_com_fmt.at("b").is_integer());
    CHECK_UNARY(x_with_com_fmt.at("c").is_string());

    CHECK_EQ(x_with_com_fmt.at("a").as_boolean(), true);
    CHECK_EQ(x_with_com_fmt.at("b").as_integer(), 42);
    CHECK_EQ(x_with_com_fmt.at("c").as_string(),  std::string("hoge"));

    CHECK_EQ(x_with_com_fmt.as_table_fmt(), fmt);

    CHECK_EQ(x_with_com_fmt.comments().size(), 2);
    CHECK_EQ(x_with_com_fmt.comments().at(0), "foo");
    CHECK_EQ(x_with_com_fmt.comments().at(1), "bar");

    CHECK_EQ(x_with_com_fmt.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_from_table(toml::table{{"foo", "bar"}}, fmt);
    x_from_table = std::map<std::string, toml::value>{{"a", true}, {"b", 42}, {"c", "hoge"}};

    test_is_type           (x_from_table, toml::value_t::table);
    test_as_type_throws    (x_from_table, toml::value_t::table);
    test_as_type_fmt_throws(x_from_table, toml::value_t::table);

    CHECK_UNARY(x_from_table.at("a").is_boolean());
    CHECK_UNARY(x_from_table.at("b").is_integer());
    CHECK_UNARY(x_from_table.at("c").is_string());

    CHECK_EQ(x_from_table.at("a").as_boolean(), true);
    CHECK_EQ(x_from_table.at("b").as_integer(), 42);
    CHECK_EQ(x_from_table.at("c").as_string(),  std::string("hoge"));

    CHECK_EQ(x_from_table.as_table_fmt(), fmt);

    CHECK_EQ(x_from_table.comments().size(), 0);

    CHECK_EQ(x_from_table.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_from_non_table(true);
    x_from_non_table = std::map<std::string, toml::value>{{"a", true}, {"b", 42}, {"c", "hoge"}};

    test_is_type           (x_from_non_table, toml::value_t::table);
    test_as_type_throws    (x_from_non_table, toml::value_t::table);
    test_as_type_fmt_throws(x_from_non_table, toml::value_t::table);

    CHECK_UNARY(x_from_non_table.at("a").is_boolean());
    CHECK_UNARY(x_from_non_table.at("b").is_integer());
    CHECK_UNARY(x_from_non_table.at("c").is_string());

    CHECK_EQ(x_from_non_table.at("a").as_boolean(), true);
    CHECK_EQ(x_from_non_table.at("b").as_integer(), 42);
    CHECK_EQ(x_from_non_table.at("c").as_string(),  std::string("hoge"));

    CHECK_EQ(x_from_non_table.comments().size(), 0);

    CHECK_EQ(x_from_non_table.location().is_ok(), false);
}

struct X
{
    toml::value into_toml() const
    {
        return toml::value(this->value);
    }
    int value;
};

inline bool operator==(const X& lhs, const X& rhs)
{
    return lhs.value == rhs.value;
}
inline bool operator!=(const X& lhs, const X& rhs)
{
    return lhs.value != rhs.value;
}

TEST_CASE("testing constructor (into_toml)")
{
    const X initialized_with{42};
    const X different_from{6*9};

    // -----------------------------------------------------------------------

    toml::value x(initialized_with);

    test_is_type(x, toml::value_t::integer);
    test_as_type_throws(x, toml::value_t::integer);
    test_as_type_fmt_throws(x, toml::value_t::integer);

    CHECK_EQ(x.as_integer(), initialized_with.value);
    CHECK_NE(x.as_integer(), different_from.value);
    CHECK_EQ(x.as_integer(std::nothrow), initialized_with.value);
    CHECK_NE(x.as_integer(std::nothrow), different_from.value);

    CHECK_EQ(x.comments().size(), 0);

    CHECK_EQ(x.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_with_comments(initialized_with, std::vector<std::string>{"foo", "bar"});

    test_is_type           (x_with_comments, toml::value_t::integer);
    test_as_type_throws    (x_with_comments, toml::value_t::integer);
    test_as_type_fmt_throws(x_with_comments, toml::value_t::integer);

    CHECK_EQ(x_with_comments.as_integer(),             initialized_with.value);
    CHECK_NE(x_with_comments.as_integer(),             different_from.value);
    CHECK_EQ(x_with_comments.as_integer(std::nothrow), initialized_with.value);
    CHECK_NE(x_with_comments.as_integer(std::nothrow), different_from.value);

    CHECK_EQ(x_with_comments.comments().size(), 2);
    CHECK_EQ(x_with_comments.comments().at(0), "foo");
    CHECK_EQ(x_with_comments.comments().at(1), "bar");

    CHECK_EQ(x_with_comments.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_from_integer(different_from);
    x_from_integer = initialized_with;

    test_is_type           (x_from_integer, toml::value_t::integer);
    test_as_type_throws    (x_from_integer, toml::value_t::integer);
    test_as_type_fmt_throws(x_from_integer, toml::value_t::integer);

    CHECK_EQ(x_from_integer.as_integer(),             initialized_with.value);
    CHECK_NE(x_from_integer.as_integer(),             different_from.value);
    CHECK_EQ(x_from_integer.as_integer(std::nothrow), initialized_with.value);
    CHECK_NE(x_from_integer.as_integer(std::nothrow), different_from.value);

    CHECK_EQ(x_from_integer.comments().size(), 0);

    CHECK_EQ(x_from_integer.location().is_ok(), false);

    // -----------------------------------------------------------------------

    toml::value x_from_non_integer(true);
    x_from_non_integer = initialized_with;

    test_is_type           (x_from_non_integer, toml::value_t::integer);
    test_as_type_throws    (x_from_non_integer, toml::value_t::integer);
    test_as_type_fmt_throws(x_from_non_integer, toml::value_t::integer);

    CHECK_EQ(x_from_non_integer.as_integer(),             initialized_with.value);
    CHECK_NE(x_from_non_integer.as_integer(),             different_from.value);
    CHECK_EQ(x_from_non_integer.as_integer(std::nothrow), initialized_with.value);
    CHECK_NE(x_from_non_integer.as_integer(std::nothrow), different_from.value);

    CHECK_EQ(x_from_non_integer.comments().size(), 0);

    CHECK_EQ(x_from_non_integer.location().is_ok(), false);
}


TEST_CASE("testing array-accessors")
{
    toml::value x({true, 42, "hoge"});

    CHECK_UNARY(x.at(0).is_boolean());
    CHECK_UNARY(x.at(1).is_integer());
    CHECK_UNARY(x.at(2).is_string());

    CHECK_UNARY(as_const(x).at(0).is_boolean());
    CHECK_UNARY(as_const(x).at(1).is_integer());
    CHECK_UNARY(as_const(x).at(2).is_string());

    CHECK_EQ(x.at(0).as_boolean(), true);
    CHECK_EQ(x.at(1).as_integer(), 42);
    CHECK_EQ(x.at(2).as_string(),  std::string("hoge"));

    CHECK_EQ(as_const(x).at(0).as_boolean(), true);
    CHECK_EQ(as_const(x).at(1).as_integer(), 42);
    CHECK_EQ(as_const(x).at(2).as_string(),  std::string("hoge"));

    CHECK_UNARY(x[0].is_boolean());
    CHECK_UNARY(x[1].is_integer());
    CHECK_UNARY(x[2].is_string());

    CHECK_EQ(x[0].as_boolean(), true);
    CHECK_EQ(x[1].as_integer(), 42);
    CHECK_EQ(x[2].as_string(),  std::string("hoge"));

    const toml::value v1(3.14);
    toml::value v2(2.71);

    x.push_back(v1);
    x.push_back(std::move(v2));

    CHECK_UNARY(x.at(3).is_floating());
    CHECK_UNARY(x.at(4).is_floating());

    CHECK_EQ(x.at(3).as_floating(), 3.14);
    CHECK_EQ(x.at(4).as_floating(), 2.71);

    x.emplace_back(6.022e23, std::vector<std::string>{"mol"});

    CHECK_UNARY(x.at(5).is_floating());
    CHECK_EQ(x.at(5).as_floating(), 6.022e23);
    CHECK_EQ(x.at(5).comments().size(), 1);
    CHECK_EQ(x.at(5).comments().at(0), "mol");

    CHECK_EQ(x.size(), 6);

    CHECK_THROWS_AS(x.at(6), std::out_of_range);
    CHECK_THROWS_AS(x.at(7), std::out_of_range);
    CHECK_THROWS_AS(as_const(x).at(6), std::out_of_range);
    CHECK_THROWS_AS(as_const(x).at(7), std::out_of_range);

    // -----------------------------------------

    toml::value non_array("foobar");
    toml::value v3("foo");
    CHECK_THROWS_AS(non_array.at(0),                    toml::type_error);
    CHECK_THROWS_AS(as_const(non_array).at(0),          toml::type_error);
    CHECK_THROWS_AS(non_array.push_back(as_const(v3)),  toml::type_error);
    CHECK_THROWS_AS(non_array.push_back(std::move(v3)), toml::type_error);
    CHECK_THROWS_AS(non_array.emplace_back(42),         toml::type_error);

    // -----------------------------------------
    toml::value string_has_size("foobar");
    CHECK_EQ(string_has_size.size(), 6);
}

TEST_CASE("testing table-accessors")
{
    toml::value x({ {"a", true}, {"b", 42}, {"c", "hoge"} });

    CHECK_UNARY(x.contains("a"));
    CHECK_UNARY(x.contains("b"));
    CHECK_UNARY(x.contains("c"));
    CHECK_UNARY_FALSE(x.contains("d"));
    CHECK_UNARY_FALSE(x.contains("e"));

    CHECK_EQ(x.count("a"), 1);
    CHECK_EQ(x.count("b"), 1);
    CHECK_EQ(x.count("c"), 1);
    CHECK_EQ(x.count("d"), 0);
    CHECK_EQ(x.count("e"), 0);

    CHECK_UNARY(x.at("a").is_boolean());
    CHECK_UNARY(x.at("b").is_integer());
    CHECK_UNARY(x.at("c").is_string());

    CHECK_EQ(x.at("a").as_boolean(), true);
    CHECK_EQ(x.at("b").as_integer(), 42);
    CHECK_EQ(x.at("c").as_string(),  std::string("hoge"));

    CHECK_UNARY(x["a"].is_boolean());
    CHECK_UNARY(x["b"].is_integer());
    CHECK_UNARY(x["c"].is_string());

    CHECK_EQ(x["a"].as_boolean(), true);
    CHECK_EQ(x["b"].as_integer(), 42);
    CHECK_EQ(x["c"].as_string(),  std::string("hoge"));

    const toml::value v1(3.14);
    x["d"] = v1;

    CHECK_UNARY(x.at("d").is_floating());
    CHECK_EQ(x.at("d").as_floating(), 3.14);

    CHECK_UNARY(x.contains("d"));
    CHECK_EQ(x.count("d"), 1);

    CHECK_EQ(x.size(), 4);

    CHECK_THROWS_AS(x.at("f"), std::out_of_range);
    CHECK_THROWS_AS(x.at("g"), std::out_of_range);
    CHECK_THROWS_AS(as_const(x).at("f"), std::out_of_range);
    CHECK_THROWS_AS(as_const(x).at("g"), std::out_of_range);

    // -----------------------------------------------

    toml::value non_table("foobar");
    CHECK_THROWS_AS(non_table.at("foo"),            toml::type_error);
    CHECK_THROWS_AS(as_const(non_table).at("foo"),  toml::type_error);
    CHECK_THROWS_AS(non_table["foo"],               toml::type_error);
    CHECK_THROWS_AS(non_table.count("foo"),         toml::type_error);
    CHECK_THROWS_AS(non_table.contains("foo"),      toml::type_error);

    toml::value no_size(42);
    CHECK_THROWS_AS(no_size.size(), toml::type_error);

    // -----------------------------------------------
    toml::value empty;
    CHECK_UNARY(empty.is_empty());

    empty["is"] = "table";

    CHECK_UNARY(empty.is_table());
    CHECK_UNARY(empty.contains("is"));
    CHECK_EQ(empty.at("is").as_string(), "table");
}

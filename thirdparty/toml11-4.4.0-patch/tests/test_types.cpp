#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/types.hpp>

TEST_CASE("testing value_t to string")
{
    CHECK_EQ(toml::to_string(toml::value_t::boolean        ), "boolean");
    CHECK_EQ(toml::to_string(toml::value_t::integer        ), "integer");
    CHECK_EQ(toml::to_string(toml::value_t::floating       ), "floating");
    CHECK_EQ(toml::to_string(toml::value_t::string         ), "string");
    CHECK_EQ(toml::to_string(toml::value_t::offset_datetime), "offset_datetime");
    CHECK_EQ(toml::to_string(toml::value_t::local_datetime ), "local_datetime");
    CHECK_EQ(toml::to_string(toml::value_t::local_date     ), "local_date");
    CHECK_EQ(toml::to_string(toml::value_t::local_time     ), "local_time");
    CHECK_EQ(toml::to_string(toml::value_t::array          ), "array");
    CHECK_EQ(toml::to_string(toml::value_t::table          ), "table");


    {std::ostringstream oss; oss << toml::value_t::boolean        ; CHECK_EQ(oss.str(), "boolean"        );}
    {std::ostringstream oss; oss << toml::value_t::integer        ; CHECK_EQ(oss.str(), "integer"        );}
    {std::ostringstream oss; oss << toml::value_t::floating       ; CHECK_EQ(oss.str(), "floating"       );}
    {std::ostringstream oss; oss << toml::value_t::string         ; CHECK_EQ(oss.str(), "string"         );}
    {std::ostringstream oss; oss << toml::value_t::offset_datetime; CHECK_EQ(oss.str(), "offset_datetime");}
    {std::ostringstream oss; oss << toml::value_t::local_datetime ; CHECK_EQ(oss.str(), "local_datetime" );}
    {std::ostringstream oss; oss << toml::value_t::local_date     ; CHECK_EQ(oss.str(), "local_date"     );}
    {std::ostringstream oss; oss << toml::value_t::local_time     ; CHECK_EQ(oss.str(), "local_time"     );}
    {std::ostringstream oss; oss << toml::value_t::array          ; CHECK_EQ(oss.str(), "array"          );}
    {std::ostringstream oss; oss << toml::value_t::table          ; CHECK_EQ(oss.str(), "table"          );}
}

#include <toml11/value.hpp>

TEST_CASE("testing type_to_enum")
{
    using value_type           = toml::value;
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

    CHECK_EQ((toml::detail::type_to_enum<boolean_type        , value_type>::value), toml::value_t::boolean        );
    CHECK_EQ((toml::detail::type_to_enum<integer_type        , value_type>::value), toml::value_t::integer        );
    CHECK_EQ((toml::detail::type_to_enum<floating_type       , value_type>::value), toml::value_t::floating       );
    CHECK_EQ((toml::detail::type_to_enum<string_type         , value_type>::value), toml::value_t::string         );
    CHECK_EQ((toml::detail::type_to_enum<local_time_type     , value_type>::value), toml::value_t::local_time     );
    CHECK_EQ((toml::detail::type_to_enum<local_date_type     , value_type>::value), toml::value_t::local_date     );
    CHECK_EQ((toml::detail::type_to_enum<local_datetime_type , value_type>::value), toml::value_t::local_datetime );
    CHECK_EQ((toml::detail::type_to_enum<offset_datetime_type, value_type>::value), toml::value_t::offset_datetime);
    CHECK_EQ((toml::detail::type_to_enum<array_type          , value_type>::value), toml::value_t::array          );
    CHECK_EQ((toml::detail::type_to_enum<table_type          , value_type>::value), toml::value_t::table          );
}

TEST_CASE("testing enum_to_type")
{
    using value_type           = toml::value;
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

    CHECK_UNARY(std::is_same<typename toml::detail::enum_to_type<toml::value_t::boolean        , value_type>::type, boolean_type        >::value);
    CHECK_UNARY(std::is_same<typename toml::detail::enum_to_type<toml::value_t::integer        , value_type>::type, integer_type        >::value);
    CHECK_UNARY(std::is_same<typename toml::detail::enum_to_type<toml::value_t::floating       , value_type>::type, floating_type       >::value);
    CHECK_UNARY(std::is_same<typename toml::detail::enum_to_type<toml::value_t::string         , value_type>::type, string_type         >::value);
    CHECK_UNARY(std::is_same<typename toml::detail::enum_to_type<toml::value_t::local_time     , value_type>::type, local_time_type     >::value);
    CHECK_UNARY(std::is_same<typename toml::detail::enum_to_type<toml::value_t::local_date     , value_type>::type, local_date_type     >::value);
    CHECK_UNARY(std::is_same<typename toml::detail::enum_to_type<toml::value_t::local_datetime , value_type>::type, local_datetime_type >::value);
    CHECK_UNARY(std::is_same<typename toml::detail::enum_to_type<toml::value_t::offset_datetime, value_type>::type, offset_datetime_type>::value);
    CHECK_UNARY(std::is_same<typename toml::detail::enum_to_type<toml::value_t::array          , value_type>::type, array_type          >::value);
    CHECK_UNARY(std::is_same<typename toml::detail::enum_to_type<toml::value_t::table          , value_type>::type, table_type          >::value);
}

TEST_CASE("testing is_exact_toml_type")
{
    using value_type           = toml::value;
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

    CHECK_UNARY(toml::detail::is_exact_toml_type<boolean_type        , value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<integer_type        , value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<floating_type       , value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<string_type         , value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<local_time_type     , value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<local_date_type     , value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<local_datetime_type , value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<offset_datetime_type, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<array_type          , value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<table_type          , value_type>::value);

    CHECK_UNARY(toml::detail::is_exact_toml_type<boolean_type        &, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<integer_type        &, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<floating_type       &, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<string_type         &, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<local_time_type     &, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<local_date_type     &, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<local_datetime_type &, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<offset_datetime_type&, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<array_type          &, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<table_type          &, value_type>::value);

    CHECK_UNARY(toml::detail::is_exact_toml_type<boolean_type         const&, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<integer_type         const&, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<floating_type        const&, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<string_type          const&, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<local_time_type      const&, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<local_date_type      const&, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<local_datetime_type  const&, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<offset_datetime_type const&, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<array_type           const&, value_type>::value);
    CHECK_UNARY(toml::detail::is_exact_toml_type<table_type           const&, value_type>::value);

    CHECK_UNARY_FALSE(toml::detail::is_exact_toml_type<float,                    value_type>::value);
    CHECK_UNARY_FALSE(toml::detail::is_exact_toml_type<std::int16_t,             value_type>::value);
    CHECK_UNARY_FALSE(toml::detail::is_exact_toml_type<std::vector<std::string>, value_type>::value);

}

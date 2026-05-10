#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/utility.hpp>

TEST_CASE("testing make_unique")
{
    const auto v = toml::cxx::make_unique<std::vector<std::string>>(std::size_t(2), "foobar");
    CHECK_EQ(v->size(), 2);
    CHECK_EQ(v->at(0),  "foobar");
    CHECK_EQ(v->at(1),  "foobar");
}

TEST_CASE("testing try_reserve")
{
    {
        // since BOOST_TEST is a macro, it cannot handle commas correctly.
        // When toml::detail::has_reserve_method<std::array<int, 1>>::value
        // is passed to a macro, C preprocessor considers
        // toml::detail::has_reserve_method<std::array<int as the first argument
        // and 1>>::value as the second argument. We need an alias to avoid
        // this problem.
        using reservable_type    = std::vector<int>  ;
        using nonreservable_type = std::array<int, 1>;
        CHECK_UNARY( toml::detail::has_reserve_method<reservable_type   >::value);
        CHECK_UNARY(!toml::detail::has_reserve_method<nonreservable_type>::value);
    }
    {
        std::vector<int> v;
        toml::detail::try_reserve(v, 100);
        CHECK_EQ(v.capacity(), 100u);
    }
    {
        std::array<int, 1> v;
        toml::detail::try_reserve(v, 100);
        CHECK_EQ(v.size(), 1);
    }
}

TEST_CASE("testing from_string")
{
    {
        const std::string str("123");
        REQUIRE_UNARY(toml::detail::from_string<int>(str).is_ok());
        CHECK_EQ(toml::detail::from_string<int>(str).unwrap(), 123);
    }
    {
        const std::string str("01");
        REQUIRE_UNARY(toml::detail::from_string<int>(str).is_ok());
        CHECK_EQ(toml::detail::from_string<int>(str).unwrap(), 1);
    }
}

TEST_CASE("testing make_string")
{
    const auto s1 = toml::detail::make_string(3, 'a');
    CHECK_EQ(s1, "aaa");

    const auto s2 = toml::detail::make_string(0, 'a');
    CHECK_EQ(s2, "");

    const std::string s("bbb");

    const auto s3 = toml::detail::make_string(s.begin(), s.end());
    CHECK_EQ(s3, "bbb");

    const auto s4 = toml::detail::make_string(s.begin(), s.begin());
    CHECK_EQ(s4, "");
}

TEST_CASE("testing make_reverse_iterator")
{
    const std::vector<int> v{1, 2, 3, 4, 5};

    const auto iter = toml::cxx::make_reverse_iterator(v.begin());
    CHECK_EQ(iter, v.rend());
}

#if defined(TOML11_HAS_STD_SOURCE_LOCATION) || defined(TOML11_HAS_EXPERIMENTAL_SOURCE_LOCATION) || defined(TOML11_HAS_BUILTIN_FILE_LINE)
TEST_CASE("cxx::source_location")
{
    const std::string file = __FILE__;
    const auto line = __LINE__; const auto loc = toml::cxx::source_location::current();

    CHECK_EQ(file, loc.file_name());
    CHECK_EQ(line, loc.line());
}
#endif

TEST_CASE("cxx::optional")
{
    {
        toml::cxx::optional<int> v(42);

        CHECK_UNARY(static_cast<bool>(v));
        CHECK_UNARY(v.has_value());

        CHECK_EQ(v.value(), 42);
        CHECK_EQ(v.value_or(6 * 9), 42);

        v.value() = 6 * 9;
        CHECK_EQ(v.value(), 54);
    }
    {
        toml::cxx::optional<int> v;

        CHECK_UNARY_FALSE(static_cast<bool>(v));
        CHECK_UNARY_FALSE(v.has_value());
        CHECK_THROWS((void)v.value());
        CHECK_EQ(v.value_or(6 * 9), 54);

        v = 42;
        CHECK_UNARY(static_cast<bool>(v));
        CHECK_UNARY(v.has_value());
        CHECK_EQ(v.value(), 42);
    }
    {
        toml::cxx::optional<int> v(toml::cxx::make_nullopt());

        CHECK_UNARY_FALSE(static_cast<bool>(v));
        CHECK_UNARY_FALSE(v.has_value());
        CHECK_THROWS((void)v.value());
        CHECK_EQ(v.value_or(6 * 9), 54);

        v = 42;
        CHECK_UNARY(static_cast<bool>(v));
        CHECK_UNARY(v.has_value());
        CHECK_EQ(v.value(), 42);
    }

    {
        toml::cxx::optional<std::vector<int>> v(std::vector<int>{42});

        CHECK_UNARY(static_cast<bool>(v));
        CHECK_UNARY(v.has_value());

        CHECK_EQ(v.value(), std::vector<int>{42});
        CHECK_EQ(v.value_or(std::vector<int>{6 * 9}), std::vector<int>{42});

        v.value() = std::vector<int>{6 * 9};
        CHECK_EQ(v.value(), std::vector<int>{54});
    }
    {
        toml::cxx::optional<std::vector<int>> v;

        CHECK_UNARY_FALSE(static_cast<bool>(v));
        CHECK_UNARY_FALSE(v.has_value());
        CHECK_THROWS((void)v.value());
        CHECK_EQ(v.value_or(std::vector<int>{6 * 9}), std::vector<int>{54});

        v = std::vector<int>{42};
        CHECK_UNARY(static_cast<bool>(v));
        CHECK_UNARY(v.has_value());
        CHECK_EQ(v.value(), std::vector<int>{42});
    }
    {
        toml::cxx::optional<std::vector<int>> v(toml::cxx::make_nullopt());

        CHECK_UNARY_FALSE(static_cast<bool>(v));
        CHECK_UNARY_FALSE(v.has_value());
        CHECK_THROWS((void)v.value());
        CHECK_EQ(v.value_or(std::vector<int>{6 * 9}), std::vector<int>{54});

        v = std::vector<int>{42};
        CHECK_UNARY(static_cast<bool>(v));
        CHECK_UNARY(v.has_value());
        CHECK_EQ(v.value(), std::vector<int>{42});
    }

}

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/storage.hpp>

TEST_CASE("testing storage construct")
{
    toml::detail::storage<int> x(42);

    REQUIRE_UNARY(x.is_ok());

    CHECK_EQ(x.get(), 42);
    CHECK_NE(x.get(), 6*9);

    x.get() = 6 * 9;

    CHECK_EQ(x.get(), 6*9);
    CHECK_NE(x.get(), 42);
}

TEST_CASE("testing storage copy")
{
    toml::detail::storage<int> x(42);
    toml::detail::storage<int> y(x);

    REQUIRE_UNARY(x.is_ok());
    REQUIRE_UNARY(y.is_ok());

    CHECK_EQ(x.get(), 42);
    CHECK_NE(x.get(), 6*9);

    CHECK_EQ(y.get(), 42);
    CHECK_NE(y.get(), 6*9);

    x.get() = 6 * 9;

    CHECK_EQ(x.get(), 6*9);
    CHECK_NE(x.get(), 42);

    CHECK_EQ(y.get(), 42);
    CHECK_NE(y.get(), 6*9);

    x = y;

    CHECK_EQ(x.get(), 42);
    CHECK_NE(x.get(), 6*9);

    CHECK_EQ(y.get(), 42);
    CHECK_NE(y.get(), 6*9);
}

TEST_CASE("testing storage move")
{
    toml::detail::storage<int> x(42);
    toml::detail::storage<int> y(x);

    REQUIRE_UNARY(x.is_ok());
    REQUIRE_UNARY(y.is_ok());

    CHECK_EQ(x.get(), 42);
    CHECK_NE(x.get(), 6*9);

    CHECK_EQ(y.get(), 42);
    CHECK_NE(y.get(), 6*9);

    x.get() = 6 * 9;

    CHECK_EQ(x.get(), 6*9);
    CHECK_NE(x.get(), 42);

    CHECK_EQ(y.get(), 42);
    CHECK_NE(y.get(), 6*9);

    x = std::move(y);

    CHECK_EQ(x.get(), 42);
    CHECK_NE(x.get(), 6*9);
}

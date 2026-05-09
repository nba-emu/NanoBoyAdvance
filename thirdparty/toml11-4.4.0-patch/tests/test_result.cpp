#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/result.hpp>

TEST_CASE("testing constructor")
{
    {
        auto s = toml::ok(42);
        toml::result<int, std::string> result(s);
        CHECK(!!result);
        CHECK(result.is_ok());
        CHECK(!result.is_err());
        CHECK(result.unwrap() == 42);
    }
    {
        const auto s = toml::ok(42);
        toml::result<int, std::string> result(s);
        CHECK(!!result);
        CHECK(result.is_ok());
        CHECK(!result.is_err());
        CHECK(result.unwrap() == 42);
    }
    {
        toml::result<int, std::string> result(toml::ok(42));
        CHECK(!!result);
        CHECK(result.is_ok());
        CHECK(!result.is_err());
        CHECK(result.unwrap() == 42);
    }

    {
        auto f = toml::err("foobar");
        toml::result<int, std::string> result(f);
        CHECK(!result);
        CHECK(!result.is_ok());
        CHECK(result.is_err());
        CHECK(result.unwrap_err() == "foobar");
    }
    {
        const auto f = toml::err("foobar");
        toml::result<int, std::string> result(f);
        CHECK(!result);
        CHECK(!result.is_ok());
        CHECK(result.is_err());
        CHECK(result.unwrap_err() == "foobar");
    }
    {
        toml::result<int, std::string> result(toml::err("foobar"));
        CHECK(!result);
        CHECK(!result.is_ok());
        CHECK(result.is_err());
        CHECK(result.unwrap_err() == "foobar");
    }
}

TEST_CASE("testing assignment op")
{
    {
        toml::result<int, std::string> result(toml::err("foobar"));
        result = toml::ok(42);
        CHECK(!!result);
        CHECK(result.is_ok());
        CHECK(!result.is_err());
        CHECK(result.unwrap() == 42);
    }
    {
        toml::result<int, std::string> result(toml::err("foobar"));
        auto s = toml::ok(42);
        result = s;
        CHECK(!!result);
        CHECK(result.is_ok());
        CHECK(!result.is_err());
        CHECK(result.unwrap() == 42);
    }
    {
        toml::result<int, std::string> result(toml::err("foobar"));
        const auto s = toml::ok(42);
        result = s;
        CHECK(!!result);
        CHECK(result.is_ok());
        CHECK(!result.is_err());
        CHECK(result.unwrap() == 42);
    }
    {
        toml::result<int, std::string> result(toml::err("foobar"));
        result = toml::err("hoge");
        CHECK(!result);
        CHECK(!result.is_ok());
        CHECK(result.is_err());
        CHECK(result.unwrap_err() == "hoge");
    }
    {
        toml::result<int, std::string> result(toml::err("foobar"));
        auto f = toml::err("hoge");
        result = f;
        CHECK(!result);
        CHECK(!result.is_ok());
        CHECK(result.is_err());
        CHECK(result.unwrap_err() == "hoge");
    }
    {
        toml::result<int, std::string> result(toml::err("foobar"));
        const auto f = toml::err("hoge");
        result = f;
        CHECK(!result);
        CHECK(!result.is_ok());
        CHECK(result.is_err());
        CHECK(result.unwrap_err() == "hoge");
    }
}

TEST_CASE("testing result<reference_wrapper>")
{
    {
        int a = 42;

        toml::result<std::reference_wrapper<int>, std::string> result(toml::ok(std::ref(a)));
        CHECK_UNARY(result);
        CHECK_UNARY(result.is_ok());
        CHECK_UNARY_FALSE(result.is_err());

        CHECK_EQ(result.unwrap(), 42);
        CHECK_EQ(a, 42);

        result.unwrap() = 6 * 9;

        CHECK_EQ(result.unwrap(), 6*9);
        CHECK_EQ(a, 6*9);
    }

    {
        std::string b = "foo";

        toml::result<int, std::reference_wrapper<std::string>> result(toml::err(std::ref(b)));
        CHECK_UNARY_FALSE(result);
        CHECK_UNARY_FALSE(result.is_ok());
        CHECK_UNARY(result.is_err());

        CHECK_EQ(result.unwrap_err(), "foo");
        CHECK_EQ(b, "foo");

        result.unwrap_err() = "foobar";

        CHECK_EQ(result.unwrap_err(), "foobar");
        CHECK_EQ(b, "foobar");
    }
}

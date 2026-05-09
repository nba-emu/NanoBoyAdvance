#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml.hpp>

TEST_CASE("testing custom error message using source_location")
{
    const toml::value root = toml::parse_str(R"(
        range = [0, 42]
        val = 54
    )");

    const auto& lower = root.at("range").at(0);
    const auto& upper = root.at("range").at(1);
    const auto& val = root.at("val");

    const auto err = toml::make_error_info("val not in range",
            lower.location(), "lower limit is defined here",
            upper.location(), "upper limit is defined here",
            val.location(),   "this is not in the range",
            "Hint: upper limit is inclusive"
        );

    CHECK_EQ(err.title(), "val not in range");
    CHECK_EQ(err.locations().size(), 3);
    CHECK_EQ(err.locations().at(0).second, "lower limit is defined here");
    CHECK_EQ(err.locations().at(1).second, "upper limit is defined here");
    CHECK_EQ(err.locations().at(2).second, "this is not in the range"   );
}

TEST_CASE("testing custom error message using value")
{
    const toml::value root = toml::parse_str(R"(
        range = [0, 42]
        val = 54
    )");

    const auto& lower = root.at("range").at(0);
    const auto& upper = root.at("range").at(1);
    const auto& val = root.at("val");

    const auto err = toml::make_error_info("val not in range",
            lower, "lower limit is defined here",
            upper, "upper limit is defined here",
            val,   "this is not in the range",
            "Hint: upper limit is inclusive"
        );

    CHECK_EQ(err.title(), "val not in range");
    CHECK_EQ(err.locations().size(), 3);
    CHECK_EQ(err.locations().at(0).second, "lower limit is defined here");
    CHECK_EQ(err.locations().at(1).second, "upper limit is defined here");
    CHECK_EQ(err.locations().at(2).second, "this is not in the range"   );
}

TEST_CASE("testing custom error message using source_location and value")
{
    const toml::value root = toml::parse_str(R"(
        range = [0, 42]
        val = 54
    )");

    const auto& lower = root.at("range").at(0);
    const auto& upper = root.at("range").at(1);
    const auto& val = root.at("val");

    const auto err = toml::make_error_info("val not in range",
            lower,          "lower limit is defined here",
            upper,          "upper limit is defined here",
            val.location(), "this is not in the range",
            "Hint: upper limit is inclusive"
        );

    CHECK_EQ(err.title(), "val not in range");
    CHECK_EQ(err.locations().size(), 3);
    CHECK_EQ(err.locations().at(0).second, "lower limit is defined here");
    CHECK_EQ(err.locations().at(1).second, "upper limit is defined here");
    CHECK_EQ(err.locations().at(2).second, "this is not in the range"   );
}

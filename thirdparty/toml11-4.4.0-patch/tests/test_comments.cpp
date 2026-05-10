#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml.hpp>

TEST_CASE("testing comments on simple value")
{
    const toml::value root = toml::parse_str(R"(
        # comment 1
        # comment 2
        a = "foo" # comment 3

        # comment 4
    )");

    const auto& a = root.at("a");
    CHECK_EQ(a.comments().size(), 3);
    CHECK_EQ(a.comments().at(0), "# comment 1");
    CHECK_EQ(a.comments().at(1), "# comment 2");
    CHECK_EQ(a.comments().at(2), "# comment 3");
}

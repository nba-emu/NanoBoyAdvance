#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/spec.hpp>

TEST_CASE("testing constructor of semantic_version")
{
    constexpr auto v123 = toml::semantic_version(1, 2, 3);

    CHECK(v123.major == 1);
    CHECK(v123.minor == 2);
    CHECK(v123.patch == 3);
}

TEST_CASE("testing the ordering of semantic_version")
{
    constexpr auto v100 = toml::semantic_version(1, 0, 0);
    constexpr auto v101 = toml::semantic_version(1, 0, 1);
    constexpr auto v110 = toml::semantic_version(1, 1, 0);
    constexpr auto v112 = toml::semantic_version(1, 1, 2);
    constexpr auto v121 = toml::semantic_version(1, 2, 1);

    CHECK(v100 == v100);
    CHECK(v100 >= v100);
    CHECK(v100 <= v100);
    CHECK_FALSE(v100 != v100);
    CHECK_FALSE(v100 >  v100);
    CHECK_FALSE(v100 <  v100);

    CHECK(v100 != v101);
    CHECK(v100 <= v101);
    CHECK(v100 <  v101);
    CHECK_FALSE(v100 == v101);
    CHECK_FALSE(v100 >= v101);
    CHECK_FALSE(v100 >  v101);

    CHECK(v101 != v110);
    CHECK(v101 <  v110);
    CHECK(v101 <= v110);
    CHECK_FALSE(v101 == v110);
    CHECK_FALSE(v101 >  v110);
    CHECK_FALSE(v101 >= v110);

    CHECK(v110 != v101);
    CHECK(v110 >  v101);
    CHECK(v110 >= v101);
    CHECK_FALSE(v110 == v101);
    CHECK_FALSE(v110 <  v101);
    CHECK_FALSE(v110 <= v101);

    CHECK(v112 != v121);
    CHECK(v112 <  v121);
    CHECK(v112 <= v121);

    CHECK(v121 != v112);
    CHECK(v121 >  v112);
    CHECK(v121 >= v112);
}

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml.hpp>

#include <clocale>

TEST_CASE("testing toml::parse(file)")
{
    using namespace toml::literals::toml_literals;

    const std::string spec_example(R"(# This is a TOML document

title = "TOML Example"

[owner]
name = "Tom Preston-Werner"
dob = 1979-05-27T07:32:00-08:00

[database]
enabled = true
ports = [ 8000, 8001, 8002 ]
data = [ ["delta", "phi"], [3.14] ]
temp_targets = { cpu = 79.5, case = 72.0 }

[servers]

[servers.alpha]
ip = "10.0.0.1"
role = "frontend"

[servers.beta]
ip = "10.0.0.2"
role = "backend"
)");

    {
        std::ofstream ofs("test_parse.toml");
        ofs << spec_example;
    }

    const auto u = toml::parse_str(spec_example);
    const auto v = toml::parse("test_parse.toml");

    CHECK_EQ(u, v);

    CHECK_EQ(toml::find<int>(v, "database", "ports", 0), 8000);
    CHECK_EQ(toml::find<int>(v, "database", "ports", 1), 8001);
    CHECK_EQ(toml::find<int>(v, "database", "ports", 2), 8002);
}

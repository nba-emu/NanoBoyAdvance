#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml.hpp>

#ifdef TOML11_ENABLE_ACCESS_CHECK

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

    const auto u = toml::parse_str(spec_example);

    CHECK_EQ(u.at("title").as_string(), "TOML Example");
    CHECK_EQ(u.at("owner").at("name").as_string(), "Tom Preston-Werner");

    CHECK_EQ(u.at("database").at("ports").at(0).as_integer(), 8000);
    CHECK_EQ(u.at("database").at("ports").at(1).as_integer(), 8001);
    CHECK_EQ(u.at("database").at("data").at(0).at(1).as_string(), "phi");

    CHECK_EQ(u.at("servers").at("alpha").at("ip"  ).as_string(), "10.0.0.1");
    CHECK_EQ(u.at("servers").at("alpha").at("role").as_string(), "frontend");

    CHECK_UNARY(u.accessed());
    CHECK_UNARY(u.at("title").accessed());
    CHECK_UNARY(u.at("owner").accessed());
    CHECK_UNARY(u.at("owner").at("name").accessed());
    CHECK_UNARY(u.at("servers").accessed());
    CHECK_UNARY(u.at("servers").at("alpha").accessed());
    CHECK_UNARY(u.at("servers").at("alpha").at("ip").accessed());
    CHECK_UNARY(u.at("servers").at("alpha").at("role").accessed());
    CHECK_UNARY(u.at("database").accessed());
    CHECK_UNARY(u.at("database").at("ports").accessed());
    CHECK_UNARY(u.at("database").at("ports").at(0).accessed());
    CHECK_UNARY(u.at("database").at("ports").at(1).accessed());

    CHECK_UNARY_FALSE(u.at("owner").at("dob").accessed());
    CHECK_UNARY_FALSE(u.at("servers").at("beta").accessed());
    CHECK_UNARY_FALSE(u.at("servers").at("beta").at("ip").accessed());
    CHECK_UNARY_FALSE(u.at("servers").at("beta").at("role").accessed());

    CHECK_UNARY_FALSE(u.at("database").at("enabled").accessed());
    CHECK_UNARY_FALSE(u.at("database").at("ports").at(2).accessed());
    CHECK_UNARY_FALSE(u.at("database").at("data").at(0).at(0).accessed());
    CHECK_UNARY_FALSE(u.at("database").at("data").at(1).at(0).accessed());
    CHECK_UNARY_FALSE(u.at("database").at("temp_targets").accessed());
    CHECK_UNARY_FALSE(u.at("database").at("temp_targets").at("cpu" ).accessed());
    CHECK_UNARY_FALSE(u.at("database").at("temp_targets").at("case").accessed());
}

#endif // TOML11_ENABLE_ACCESS_CHECK

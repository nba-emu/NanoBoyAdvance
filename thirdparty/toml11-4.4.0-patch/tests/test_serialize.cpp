#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml.hpp>

#include <clocale>

TEST_CASE("testing serialization")
{
    using namespace toml::literals::toml_literals;

    const auto spec_example = R"(# This is a TOML document

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
role = "backend")"_toml;

    // format and parse
    const auto v = toml::parse_str(toml::format(spec_example));

    CHECK_EQ(v, spec_example);
}

TEST_CASE("testing serialization with complicated keys")
{
    using namespace toml::literals::toml_literals;

    const auto spec_example = R"(
[keys]

key = "value"
bare_key = "value"
bare-key = "value"
1234 = "value"

"127.0.0.1"          = "value"
"character encoding" = "value"
"ʎǝʞ"                = "value"
'key2'               = "value"
'quoted "value"'     = "value"

"" = "blank"

fruits.apple.skin = "thin"
fruits.apple.color = "red"

fruits.orange.skin = "thick"
fruits.orange.color = "orange"

site."google.com" = true
3.14159 = "pi"
)"_toml;

    // format and parse
    const auto v = toml::parse_str(toml::format(spec_example));

    CHECK_EQ(v, spec_example);
}

TEST_CASE("testing serialization with array of tables")
{
    using namespace toml::literals::toml_literals;

    const auto spec_example = R"(
points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ]

[[products]]
name = "Hammer"
sku = 738594937

[[products]]  # empty table within the array

[[products]]
name = "Nail"
sku = 284758393

color = "gray"

[[fruits]]
name = "apple"

[fruits.physical]  # subtable
color = "red"
shape = "round"

[[fruits.varieties]]  # nested array of tables
name = "red delicious"

[[fruits.varieties]]
name = "granny smith"

[[fruits]]
name = "banana"

[[fruits.varieties]]
name = "plantain"
)"_toml;

    // format and parse
    const auto v = toml::parse_str(toml::format(spec_example));

    CHECK_EQ(v, spec_example);
}

TEST_CASE("testing serialization with locale")
{
    std::string current_locale = std::setlocale(LC_ALL, nullptr);

    // fr_FR is a one of locales that uses `,` as a decimal separator.
    if(const char* try_hyphen = std::setlocale(LC_ALL, "fr_FR.UTF-8"))
    {
        current_locale = std::string(try_hyphen);
    }
    else if(const char* try_nohyphen = std::setlocale(LC_ALL, "fr_FR.utf8"))
    {
        current_locale = std::string(try_nohyphen);
    }

    MESSAGE("current_locale = ", current_locale);

    const auto v1 = toml::parse_str(R"(
        pi        = 3.1415_9265
        large_int = 123_456_789
    )");
    const auto v2 = toml::parse_str(toml::format(v1));

    // actually, it checkl if v1 is serialized correctly under FR locale
    // where 3.1415 -> 3,1415
    CHECK_EQ(v1, v2);
}

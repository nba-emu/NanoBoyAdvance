#include <toml.hpp>

#include <iostream>

#include <cassert>

int main()
{
    const auto root = toml::parse("key_example.toml");

    assert(root.is_table());
    assert(root.comments().size() == 2);
    assert(root.comments().at(0) == " This is a TOML document.");
    assert(root.comments().at(1) == " This contains most of the examples in the spec (from https://toml.io).");

    assert(root.size() == 1);
    assert(root.contains("key"));
    assert(root.at("key").is_table());

    // using member functions
    {
        const toml::value& keys = root.at("key");
        assert(keys.at("key"     ).as_string() == "value");
        assert(keys.at("bare_key").as_string() == "value");
        assert(keys.at("bare-key").as_string() == "value");
        assert(keys.at("1234"    ).as_string() == "value");

        assert(keys.at("127.0.0.1"         ).as_string() == "value");
        assert(keys.at("character encoding").as_string() == "value");
        assert(keys.at("ʎǝʞ"               ).as_string() == "value"); // requires /utf-8 in MSVC
        assert(keys.at("key2"              ).as_string() == "value");
        assert(keys.at("quoted \"value\""  ).as_string() == "value");

        assert(keys.at("").as_string() == "blank");

        assert(keys.at("fruit").at("apple").at("skin" ).as_string() == "thin");
        assert(keys.at("fruit").at("apple").at("color").as_string() == "red");
        assert(keys.at("fruit").at("orange").at("skin" ).as_string() == "thick");
        assert(keys.at("fruit").at("orange").at("color").as_string() == "orange");

        assert(keys.at("site").at("google.com").as_boolean() == true);
        assert(keys.at("3").at("14159").as_string() == "pi");
    }

    // using toml::find
    {
        assert(toml::find<std::string>(root, "keys", "key"     ) == "value");
        assert(toml::find<std::string>(root, "keys", "bare_key") == "value");
        assert(toml::find<std::string>(root, "keys", "bare-key") == "value");
        assert(toml::find<std::string>(root, "keys", "1234"    ) == "value");

        const toml::value& keys = toml::find(root, "keys");

        assert(toml::find<std::string>(keys, "127.0.0.1"         ) == "value");
        assert(toml::find<std::string>(keys, "character encoding") == "value");
        assert(toml::find<std::string>(keys, "ʎǝʞ"               ) == "value"); // requires /utf-8 in MSVC
        assert(toml::find<std::string>(keys, "key2"              ) == "value");
        assert(toml::find<std::string>(keys, "quoted \"value\""  ) == "value");

        assert(toml::find<std::string>(keys, "") == "blank");

        assert(toml::find<std::string>(keys, "fruit", "apple" , "skin" ) == "thin");
        assert(toml::find<std::string>(keys, "fruit", "apple" , "color") == "red");
        assert(toml::find<std::string>(keys, "fruit", "orange", "skin" ) == "thick");
        assert(toml::find<std::string>(keys, "fruit", "orange", "color") == "orange");

        assert(toml::find<bool>(keys, "site", "google.com") == true);
        assert(toml::find<std::string>(keys, "3", "14159") == "pi");
    }

    std::cout << "ok." << std::endl;

    return 0;
}

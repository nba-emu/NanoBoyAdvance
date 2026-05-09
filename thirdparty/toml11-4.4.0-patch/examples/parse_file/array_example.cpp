#include <toml.hpp>

#include <iostream>

#include <cassert>

int main()
{
    const auto root = toml::parse("array_example.toml");

    // using member functions
    {
        assert(root.at("integers").is_array());
        assert(root.at("integers").at(0).as_integer() == 1);
        assert(root.at("integers").at(1).as_integer() == 2);
        assert(root.at("integers").at(2).as_integer() == 3);

        assert(root.at("colors").at(0).as_string() == "red");
        assert(root.at("colors").at(1).as_string() == "yellow");
        assert(root.at("colors").at(2).as_string() == "green");

        assert(root.at("nested_arrays_of_ints").at(0).at(0).as_integer() == 1);
        assert(root.at("nested_arrays_of_ints").at(0).at(1).as_integer() == 2);
        assert(root.at("nested_arrays_of_ints").at(1).at(0).as_integer() == 3);
        assert(root.at("nested_arrays_of_ints").at(1).at(1).as_integer() == 4);
        assert(root.at("nested_arrays_of_ints").at(1).at(2).as_integer() == 5);

        assert(root.at("nested_mixed_array").at(0).at(0).as_integer() == 1);
        assert(root.at("nested_mixed_array").at(0).at(1).as_integer() == 2);
        assert(root.at("nested_mixed_array").at(1).at(0).as_string() == "a");
        assert(root.at("nested_mixed_array").at(1).at(1).as_string() == "b");
        assert(root.at("nested_mixed_array").at(1).at(2).as_string() == "c");

        assert(root.at("string_array").at(0).as_string() == "all");
        assert(root.at("string_array").at(1).as_string() == "strings");
        assert(root.at("string_array").at(2).as_string() == "are the same");
        assert(root.at("string_array").at(3).as_string() == "type");

        assert(root.at("numbers").at(0).as_floating() == std::stod("0.1"));
        assert(root.at("numbers").at(1).as_floating() == std::stod("0.2"));
        assert(root.at("numbers").at(2).as_floating() == std::stod("0.5"));
        assert(root.at("numbers").at(3).as_integer()  == 1);
        assert(root.at("numbers").at(4).as_integer()  == 2);
        assert(root.at("numbers").at(5).as_integer()  == 5);

        assert(root.at("contributors").at(0).as_string() == "Foo Bar <foo@example.com>");
        assert(root.at("contributors").at(1).at("name" ).as_string() == "Baz Qux");
        assert(root.at("contributors").at(1).at("email").as_string() == "bazqux@example.com");
        assert(root.at("contributors").at(1).at("url"  ).as_string() == "https://example.com/bazqux");
    }

    // using toml::find
    {
        assert(toml::find<int>(root, "integers", 0) == 1);
        assert(toml::find<int>(root, "integers", 1) == 2);
        assert(toml::find<int>(root, "integers", 2) == 3);

        const auto integers = toml::find<std::vector<int>>(root, "integers");
        assert(integers.at(0) == 1);
        assert(integers.at(1) == 2);
        assert(integers.at(2) == 3);

        assert(toml::find<std::string>(root, "colors", 0) == "red");
        assert(toml::find<std::string>(root, "colors", 1) == "yellow");
        assert(toml::find<std::string>(root, "colors", 2) == "green");

        const auto colors = toml::find<std::array<std::string, 3>>(root, "colors");
        assert(colors.at(0) == "red");
        assert(colors.at(1) == "yellow");
        assert(colors.at(2) == "green");

        assert(toml::find<int>(root, "nested_arrays_of_ints", 0, 0) == 1);
        assert(toml::find<int>(root, "nested_arrays_of_ints", 0, 1) == 2);
        assert(toml::find<int>(root, "nested_arrays_of_ints", 1, 0) == 3);
        assert(toml::find<int>(root, "nested_arrays_of_ints", 1, 1) == 4);
        assert(toml::find<int>(root, "nested_arrays_of_ints", 1, 2) == 5);

        const auto nested_arrays_of_ints = toml::find<std::vector<std::vector<int>>>(root, "nested_arrays_of_ints");
        assert(nested_arrays_of_ints.at(0).at(0) == 1);
        assert(nested_arrays_of_ints.at(0).at(1) == 2);
        assert(nested_arrays_of_ints.at(1).at(0) == 3);
        assert(nested_arrays_of_ints.at(1).at(1) == 4);
        assert(nested_arrays_of_ints.at(1).at(2) == 5);

        assert(toml::find<int        >(root, "nested_mixed_array", 0, 0) == 1);
        assert(toml::find<int        >(root, "nested_mixed_array", 0, 1) == 2);
        assert(toml::find<std::string>(root, "nested_mixed_array", 1, 0) == "a");
        assert(toml::find<std::string>(root, "nested_mixed_array", 1, 1) == "b");
        assert(toml::find<std::string>(root, "nested_mixed_array", 1, 2) == "c");

        const auto nested_mixed_array = toml::find<
                std::pair<std::vector<int>, std::vector<std::string>>
            >(root, "nested_mixed_array");

        assert(nested_mixed_array.first .at(0) == 1);
        assert(nested_mixed_array.first .at(1) == 2);
        assert(nested_mixed_array.second.at(0) == "a");
        assert(nested_mixed_array.second.at(1) == "b");
        assert(nested_mixed_array.second.at(2) == "c");

        assert(toml::find<std::string>(root, "string_array", 0) == "all");
        assert(toml::find<std::string>(root, "string_array", 1) == "strings");
        assert(toml::find<std::string>(root, "string_array", 2) == "are the same");
        assert(toml::find<std::string>(root, "string_array", 3) == "type");

        const auto numbers = toml::find<
            std::tuple<double, double, double, int, int, int>
            >(root, "numbers");

        assert(std::get<0>(numbers) == std::stod("0.1"));
        assert(std::get<1>(numbers) == std::stod("0.2"));
        assert(std::get<2>(numbers) == std::stod("0.5"));
        assert(std::get<3>(numbers) == 1);
        assert(std::get<4>(numbers) == 2);
        assert(std::get<5>(numbers) == 5);

        struct contributor_t
        {
            contributor_t(const toml::value& v)
            {
                if(v.is_string())
                {
                    name = v.as_string();
                }
                else
                {
                    assert(v.is_table());
                    name  = toml::find<std::string>(v, "name");
                    email = toml::find_or(v, "email", std::string(""));
                    url   = toml::find_or(v, "url"  , std::string(""));
                }
            }

            std::string name;
            std::string email;
            std::string url;
        };

        const auto contributors = toml::find<std::vector<contributor_t>>(root, "contributors");

        assert(contributors.at(0).name  == "Foo Bar <foo@example.com>");
        assert(contributors.at(0).email == "");
        assert(contributors.at(0).url   == "");
        assert(contributors.at(1).name  == "Baz Qux");
        assert(contributors.at(1).email == "bazqux@example.com");
        assert(contributors.at(1).url   == "https://example.com/bazqux");
    }

    std::cout << "ok." << std::endl;

    return 0;
}

#include <toml.hpp>

#include <iostream>
#include <map>

#include <cassert>

int main()
{
    const auto root = toml::parse("array_of_tables_example.toml");

    // using member functions
    {
        assert(root.at("points").is_array());
        assert(root.at("points").at(0).is_table());
        assert(root.at("points").at(1).is_table());
        assert(root.at("points").at(2).is_table());

        assert(root.at("points").at(0).at("x").as_integer() == 1);
        assert(root.at("points").at(0).at("y").as_integer() == 2);
        assert(root.at("points").at(0).at("z").as_integer() == 3);

        assert(root.at("points").at(1).at("x").as_integer() == 7);
        assert(root.at("points").at(1).at("y").as_integer() == 8);
        assert(root.at("points").at(1).at("z").as_integer() == 9);

        assert(root.at("points").at(2).at("x").as_integer() == 2);
        assert(root.at("points").at(2).at("y").as_integer() == 4);
        assert(root.at("points").at(2).at("z").as_integer() == 8);


        assert(root.at("products").at(0).at("name").as_string()  == "Hammer");
        assert(root.at("products").at(0).at("sku" ).as_integer() == 738594937);

        assert(root.at("products").at(1).as_table().empty());

        assert(root.at("products").at(2).at("name" ).as_string()  == "Nail");
        assert(root.at("products").at(2).at("sku"  ).as_integer() == 284758393);
        assert(root.at("products").at(2).at("color").as_string()  == "gray");

        assert(root.at("fruits").at(0).at("name").as_string()  == "apple");
        assert(root.at("fruits").at(0).at("physical").at("color").as_string()  == "red");
        assert(root.at("fruits").at(0).at("physical").at("shape").as_string()  == "round");
        assert(root.at("fruits").at(0).at("varieties").at(0).at("name").as_string()  == "red delicious");
        assert(root.at("fruits").at(0).at("varieties").at(1).at("name").as_string()  == "granny smith");

        assert(root.at("fruits").at(1).at("name").as_string()  == "banana");
        assert(root.at("fruits").at(1).at("varieties").at(0).at("name").as_string()  == "plantain");
    }

    // using toml::find
    {
        assert(toml::find<int>(root, "points", 0, "x") == 1);
        assert(toml::find<int>(root, "points", 0, "y") == 2);
        assert(toml::find<int>(root, "points", 0, "z") == 3);

        assert(toml::find<int>(root, "points", 1, "x") == 7);
        assert(toml::find<int>(root, "points", 1, "y") == 8);
        assert(toml::find<int>(root, "points", 1, "z") == 9);

        assert(toml::find<int>(root, "points", 2, "x") == 2);
        assert(toml::find<int>(root, "points", 2, "y") == 4);
        assert(toml::find<int>(root, "points", 2, "z") == 8);

        const auto points = toml::find<
                std::vector<std::map<std::string, int>>
            >(root, "points");

        assert(points.at(0).at("x") == 1);
        assert(points.at(0).at("y") == 2);
        assert(points.at(0).at("z") == 3);

        assert(points.at(1).at("x") == 7);
        assert(points.at(1).at("y") == 8);
        assert(points.at(1).at("z") == 9);

        assert(points.at(2).at("x") == 2);
        assert(points.at(2).at("y") == 4);
        assert(points.at(2).at("z") == 8);

        struct product_t
        {
            product_t(const toml::value& v)
                : name (toml::find_or<std::string  >(v, "name", "")),
                  sku  (toml::find_or<std::uint64_t>(v, "sku", 0)),
                  color(toml::find_or<std::string  >(v, "color", ""))
            {}
            std::string   name;
            std::uint64_t sku;
            std::string   color;
        };

        const auto products = toml::find<std::vector<product_t>>(root, "products");

        assert(products.at(0).name  == "Hammer");
        assert(products.at(0).sku   == 738594937);
        assert(products.at(0).color == "");

        assert(products.at(1).name  == "");
        assert(products.at(1).sku   == 0);
        assert(products.at(1).color == "");

        assert(products.at(2).name  == "Nail");
        assert(products.at(2).sku   == 284758393);
        assert(products.at(2).color == "gray");

        struct fruit_t
        {
            fruit_t(const toml::value& v)
                : name(toml::find<std::string>(v, "name")),
                  physical (toml::find<std::map<std::string, std::string>>(v, "physical")),
                  varieties(toml::find<std::vector<std::map<std::string, std::string>>>(v, "varieties"))
            {}

            std::string name;
            std::map<std::string, std::string> physical;
            std::vector<std::map<std::string, std::string>> varieties;
        };

        const auto fruits = toml::find<std::vector<fruit_t>>(root, "fruits");

        assert(fruits.at(0).name == "apple");
        assert(fruits.at(0).physical.at("color") == "red");
        assert(fruits.at(0).physical.at("shape") == "round");
        assert(fruits.at(0).varieties.at(0).at("name") == "red delicious");
        assert(fruits.at(0).varieties.at(1).at("name") == "granny smith");

        assert(fruits.at(1).name == "banana");
        assert(fruits.at(1).varieties.at(0).at("name") == "plantain");
    }

    std::cout << "ok." << std::endl;

    return 0;
}

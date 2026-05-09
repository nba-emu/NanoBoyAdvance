#include <toml.hpp>

#include <iostream>
#include <map>

#include <cassert>

int main()
{
    const auto root = toml::parse("spec_example.toml");

    // using member functions
    {
        assert(root.at("title").as_string() == "TOML Example");

        assert(root.at("owner").at("name").as_string() == "Tom Preston-Werner");

        const auto dob = root.at("owner").at("dob" ).as_offset_datetime();
        assert(dob.date  .year   ==  1979);
        assert(dob.date  .month  ==  static_cast<int>(toml::month_t::May));
        assert(dob.date  .day    ==  27);
        assert(dob.time  .hour   ==  7);
        assert(dob.time  .minute ==  32);
        assert(dob.time  .second ==  0);
        assert(dob.offset.hour   == -8);
        assert(dob.offset.minute ==  0);

        assert(root.at("database").at("enabled").as_boolean());
        assert(root.at("database").at("ports").at(0).as_integer() == 8000);
        assert(root.at("database").at("ports").at(1).as_integer() == 8001);
        assert(root.at("database").at("ports").at(2).as_integer() == 8002);
        assert(root.at("database").at("data").at(0).at(0).as_string()   == "delta");
        assert(root.at("database").at("data").at(0).at(1).as_string()   == "phi");
        assert(root.at("database").at("data").at(1).at(0).as_floating() == 3.14);
        assert(root.at("database").at("temp_targets").at("cpu" ).as_floating() == 79.5);
        assert(root.at("database").at("temp_targets").at("case").as_floating() == 72.0);

        assert(root.at("servers").at("alpha").at("ip"  ).as_string() == "10.0.0.1");
        assert(root.at("servers").at("alpha").at("role").as_string() == "frontend");
        assert(root.at("servers").at("beta" ).at("ip"  ).as_string() == "10.0.0.2");
        assert(root.at("servers").at("beta" ).at("role").as_string() == "backend");
    }

    // using toml::find
    {
        assert(toml::find<std::string>(root, "title") == "TOML Example");

        assert(toml::find<std::string>(root, "owner", "name") == "Tom Preston-Werner");

        const auto dob = toml::find<toml::offset_datetime>(root, "owner", "dob");
        assert(dob.date  .year   ==  1979);
        assert(dob.date  .month  ==  static_cast<int>(toml::month_t::May));
        assert(dob.date  .day    ==  27);
        assert(dob.time  .hour   ==  7);
        assert(dob.time  .minute ==  32);
        assert(dob.time  .second ==  0);
        assert(dob.offset.hour   == -8);
        assert(dob.offset.minute ==  0);

        assert(toml::find<bool>(root, "database", "enabled"));

        const auto ports = toml::find<std::vector<int>>(root, "database", "ports");
        assert(ports.at(0) == 8000);
        assert(ports.at(1) == 8001);
        assert(ports.at(2) == 8002);

        const auto data = toml::find<std::pair<std::vector<std::string>, std::vector<double>>>(root, "database", "data");
        assert(data.first.at(0)  == "delta");
        assert(data.first.at(1)  == "phi");
        assert(data.second.at(0) == 3.14);

        const auto temp_targets = toml::find<std::map<std::string, double>>(root, "database", "temp_targets");
        assert(temp_targets.at("cpu" ) == 79.5);
        assert(temp_targets.at("case") == 72.0);

        const auto servers = toml::find<std::map<std::string, std::map<std::string, std::string>>>(root, "servers");
        assert(servers.at("alpha").at("ip"  ) == "10.0.0.1");
        assert(servers.at("alpha").at("role") == "frontend");
        assert(servers.at("beta" ).at("ip"  ) == "10.0.0.2");
        assert(servers.at("beta" ).at("role") == "backend" );
    }

    std::cout << "ok." << std::endl;

    return 0;
}

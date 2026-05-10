#include <toml.hpp>

#include <iostream>
#include <map>

#include <cassert>

struct u8config
{
    using comment_type  = toml::preserve_comments;

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::u8string; // XXX

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    using table_type = std::unordered_map<K, T>;

    static toml::result<integer_type, toml::error_info>
    parse_int(const std::string& str, const toml::source_location src, const std::uint8_t base)
    {
        return toml::read_int<integer_type>(str, src, base);
    }
    static toml::result<floating_type, toml::error_info>
    parse_float(const std::string& str, const toml::source_location src, const bool is_hex)
    {
        return toml::read_float<floating_type>(str, src, is_hex);
    }
};

int main()
{
    const auto root = toml::parse<u8config>("spec_example.toml");

    // using member functions
    {
        assert(root.at(u8"title").as_string() == u8"TOML Example");

        assert(root.at(u8"owner").at(u8"name").as_string() == u8"Tom Preston-Werner");

        const auto dob = root.at(u8"owner").at(u8"dob" ).as_offset_datetime();
        assert(dob.date  .year   ==  1979);
        assert(dob.date  .month  ==  static_cast<int>(toml::month_t::May));
        assert(dob.date  .day    ==  27);
        assert(dob.time  .hour   ==  7);
        assert(dob.time  .minute ==  32);
        assert(dob.time  .second ==  0);
        assert(dob.offset.hour   == -8);
        assert(dob.offset.minute ==  0);

        assert(root.at(u8"database").at(u8"enabled").as_boolean());
        assert(root.at(u8"database").at(u8"ports").at(0).as_integer() == 8000);
        assert(root.at(u8"database").at(u8"ports").at(1).as_integer() == 8001);
        assert(root.at(u8"database").at(u8"ports").at(2).as_integer() == 8002);
        assert(root.at(u8"database").at(u8"data").at(0).at(0).as_string()   == u8"delta");
        assert(root.at(u8"database").at(u8"data").at(0).at(1).as_string()   == u8"phi");
        assert(root.at(u8"database").at(u8"data").at(1).at(0).as_floating() == 3.14);
        assert(root.at(u8"database").at(u8"temp_targets").at(u8"cpu" ).as_floating() == 79.5);
        assert(root.at(u8"database").at(u8"temp_targets").at(u8"case").as_floating() == 72.0);

        assert(root.at(u8"servers").at(u8"alpha").at(u8"ip"  ).as_string() == u8"10.0.0.1");
        assert(root.at(u8"servers").at(u8"alpha").at(u8"role").as_string() == u8"frontend");
        assert(root.at(u8"servers").at(u8"beta" ).at(u8"ip"  ).as_string() == u8"10.0.0.2");
        assert(root.at(u8"servers").at(u8"beta" ).at(u8"role").as_string() == u8"backend");
    }

    // using toml::find
    {
        // you can get as std::string from u8string, using toml::get/find
        assert(toml::find<std::string>(root, u8"title") == "TOML Example");

        assert(toml::find<std::string>(root, u8"owner", u8"name") == "Tom Preston-Werner");

        const auto dob = toml::find<toml::offset_datetime>(root, u8"owner", u8"dob");
        assert(dob.date  .year   ==  1979);
        assert(dob.date  .month  ==  static_cast<int>(toml::month_t::May));
        assert(dob.date  .day    ==  27);
        assert(dob.time  .hour   ==  7);
        assert(dob.time  .minute ==  32);
        assert(dob.time  .second ==  0);
        assert(dob.offset.hour   == -8);
        assert(dob.offset.minute ==  0);

        assert(toml::find<bool>(root, u8"database", u8"enabled"));

        const auto ports = toml::find<std::vector<int>>(root, u8"database", u8"ports");
        assert(ports.at(0) == 8000);
        assert(ports.at(1) == 8001);
        assert(ports.at(2) == 8002);

        const auto data = toml::find<std::pair<std::vector<std::string>, std::vector<double>>>(root, u8"database", u8"data");
        assert(data.first.at(0)  == "delta");
        assert(data.first.at(1)  == "phi");
        assert(data.second.at(0) == 3.14);

        const auto temp_targets = toml::find<std::map<std::string, double>>(root, u8"database", u8"temp_targets");
        assert(temp_targets.at("cpu" ) == 79.5);
        assert(temp_targets.at("case") == 72.0);

        const auto servers = toml::find<std::map<std::string, std::map<std::string, std::string>>>(root, u8"servers");
        assert(servers.at("alpha").at("ip"  ) == "10.0.0.1");
        assert(servers.at("alpha").at("role") == "frontend");
        assert(servers.at("beta" ).at("ip"  ) == "10.0.0.2");
        assert(servers.at("beta" ).at("role") == "backend" );
    }

    const std::u8string out = toml::format(root);

    std::string printable;
    std::transform(out.begin(), out.end(), std::back_inserter(printable),
            [](const char8_t c) {return static_cast<char>(c);});
    std::cout << printable << std::endl;

    std::cout << "ok." << std::endl;

    return 0;
}

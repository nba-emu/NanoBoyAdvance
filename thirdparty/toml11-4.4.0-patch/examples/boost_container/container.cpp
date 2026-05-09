#include <boost/container/small_vector.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

#include <toml.hpp>

#include <iostream>

struct boost_config
{
    using comment_type  = toml::preserve_comments;

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = boost::container::small_vector<T, 8>;
    template<typename K, typename T>
    using table_type = boost::unordered_flat_map<K, T>;

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

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cerr << "usage: ./container <input.toml>" << std::endl;
        return 1;
    }

    const auto input = toml::parse_str<boost_config>(argv[1]);

    std::cout << toml::format(input) << std::endl;

    return 0;
}

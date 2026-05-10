#include <boost/multiprecision/number.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>

#include <toml.hpp>

struct large_num_config
{
    using comment_type  = toml::preserve_comments;

    using boolean_type  = bool;
    using integer_type  = boost::multiprecision::cpp_int;
    using floating_type = boost::multiprecision::cpp_bin_float_oct;
    using string_type   = std::string;

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
    const std::string input_str(R"(
large_int_dec = 10_000_000_000_000_000_000
large_int_hex = 0x0001_0000_0000_0000_0000
large_float_pi = 3.1415926535897932384626433832795028842
    )");

    const auto input = toml::parse_str<large_num_config>(input_str);

    std::cout << "int64_t max    = " << (std::numeric_limits<std::int64_t>::max)() << std::endl;

    std::cout << "large_int_dec  = " << input.at("large_int_dec" ).as_integer() << std::endl;
    std::cout << "large_int_hex  = " << input.at("large_int_hex" ).as_integer() << std::endl;
    std::cout << "large_float_pi = "
        << std::setprecision(std::numeric_limits<boost::multiprecision::cpp_bin_float_oct>::max_digits10 - 1)
        << input.at("large_float_pi").as_floating() << std::endl;

    std::cout << "=================" << std::endl;
    std::cout << toml::format(input) << std::endl;

    return 0;
}

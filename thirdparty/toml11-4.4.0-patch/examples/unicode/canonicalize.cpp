#include <toml.hpp>
#include <uni_algo/norm.h>

#include <iostream>
#include <map>

struct nfc_comparator
{
    using first_argument_type  = std::string;
    using second_argument_type = std::string;
    using result_type          = bool;

    result_type operator()(const first_argument_type& lhs, const second_argument_type& rhs) const
    {
        return una::norm::to_nfc_utf8(lhs) < una::norm::to_nfc_utf8(rhs);
    }
};

struct nfc_equal_to
{
    using first_argument_type  = std::string;
    using second_argument_type = std::string;
    using result_type          = bool;

    result_type operator()(const first_argument_type& lhs, const second_argument_type& rhs) const
    {
        return una::norm::to_nfc_utf8(lhs) == una::norm::to_nfc_utf8(rhs);
    }
};

struct nfc_hasher
{
    std::size_t operator()(const std::string& s) const
    {
        return std::hash<std::string>{}(una::norm::to_nfc_utf8(s));
    }
};

struct nfc_config
{
    using comment_type  = toml::preserve_comments;

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    // using table_type = std::map<K, T, nfc_comparator>;
    using table_type = std::unordered_map<K, T, nfc_hasher, nfc_equal_to>;

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


int main(int argc, char **argv)
{
    if(argc != 2)
    {
        return 1;
    }

    const auto input = toml::parse<nfc_config>(argv[1]);

    std::cout << toml::format(input) << std::endl;

    return 0;
}

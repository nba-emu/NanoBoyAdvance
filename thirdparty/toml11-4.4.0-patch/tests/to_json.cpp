
#include <iomanip>
#include <iostream>
#include <string>

#include <toml.hpp>
#include <nlohmann/json.hpp>

#include <cmath>

#ifdef TOML11_TO_JSON_USE_V1_1_0
#  define TOML11_TO_JSON_SPEC toml::spec::v(1, 1, 0)
#else
#  define TOML11_TO_JSON_SPEC toml::spec::v(1, 0, 0)
#endif

template<typename Value = toml::value>
struct json_converter
{
    using value_type = Value;
    using boolean_type         = typename value_type::boolean_type        ;
    using integer_type         = typename value_type::integer_type        ;
    using floating_type        = typename value_type::floating_type       ;
    using string_type          = typename value_type::string_type         ;
    using local_time_type      = typename value_type::local_time_type     ;
    using local_date_type      = typename value_type::local_date_type     ;
    using local_datetime_type  = typename value_type::local_datetime_type ;
    using offset_datetime_type = typename value_type::offset_datetime_type;
    using array_type           = typename value_type::array_type          ;
    using table_type           = typename value_type::table_type          ;

    nlohmann::json operator()(boolean_type v)
    {
        nlohmann::json j;
        j["type"] = "bool";
        if(v)
        {
            j["value"] = "true";
        }
        else
        {
            j["value"] = "false";
        }
        return j;
    }
    nlohmann::json operator()(integer_type v)
    {
        nlohmann::json j;
        j["type"] = "integer";
        j["value"] = std::to_string(v);
        return j;
    }
    nlohmann::json operator()(floating_type v)
    {
        std::ostringstream oss;
        oss << std::setprecision(16);
        // if we set the precision as max_digit, 1.1 will be 1.1000000000000001.
        // But toml-test does not allow 1.1000000000000001.
        if(std::isnan(v) && std::signbit(v))
        {
            // toml-test does not allow negative NaN represented in "-nan" because
            // there are languages that does not distinguish nan and -nan.
            // But toml11 keeps sign from input. To resolve this difference,
            // we convert -nan to nan here.
            v = std::numeric_limits<floating_type>::quiet_NaN();
        }
        oss << v;

        nlohmann::json j;
        j["type"] = "float";
        j["value"] = oss.str();
        return j;
    }
    nlohmann::json operator()(const string_type& v)
    {
        nlohmann::json j;
        j["type"] = "string";
        j["value"] = v;
        return j;
    }
    nlohmann::json operator()(const toml::local_time& v)
    {
        nlohmann::json j;
        j["type"] = "time-local";
        j["value"] = toml::to_string(v);
        return j;
    }
    nlohmann::json operator()(const toml::local_date& v)
    {
        nlohmann::json j;
        j["type"] = "date-local";
        j["value"] = toml::to_string(v);
        return j;
    }
    nlohmann::json operator()(const toml::local_datetime& v)
    {
        nlohmann::json j;
        j["type"] = "datetime-local";
        j["value"] = toml::to_string(v);
        return j;
    }
    nlohmann::json operator()(const toml::offset_datetime& v)
    {
        nlohmann::json j;
        j["type"] = "datetime";
        j["value"] = toml::to_string(v);
        return j;
    }
    nlohmann::json operator()(const toml::array& v)
    {
        nlohmann::json j = nlohmann::json::array();
        for(const auto& elem : v)
        {
            j.push_back(toml::visit(*this, elem));
        }
        return j;
    }
    nlohmann::json operator()(const toml::table& v)
    {
        nlohmann::json j = nlohmann::json::object();
        for(const auto& kv : v)
        {
            j[kv.first] = toml::visit(*this, kv.second);
        }
        return j;
    }
};

int main(int argc, char** argv)
{
    try
    {
        if(argc == 2)
        {
            const std::string fname(argv[1]);
            const auto data = toml::parse(fname, TOML11_TO_JSON_SPEC);
            std::cout << toml::visit(json_converter<>(), data);
            return 0;
        }
        else
        {
            std::vector<unsigned char> buf;
            std::cin.peek();

            while(!std::cin.eof())
            {
                buf.push_back(static_cast<unsigned char>(std::cin.get()));
                std::cin.peek();
            }

            const auto data = toml::parse(buf, "cin", TOML11_TO_JSON_SPEC);
            std::cout << toml::visit(json_converter<>(), data);
            return 0;
        }
    }
    catch(const std::exception& err)
    {
        std::cout << "what(): " << err.what() << std::endl;
        return 1;
    }
}

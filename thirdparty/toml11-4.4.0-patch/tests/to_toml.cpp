#include <iostream>
#include <string>

#include <toml.hpp>
#include <nlohmann/json.hpp>

#include <cmath>

toml::value convert_to_toml(const nlohmann::json& j)
{
    if(j.is_array())
    {
        toml::value a(toml::array{});
        for(const auto& v : j)
        {
            a.push_back(convert_to_toml(v));
        }
        return a;
    }
    else if(j.size() == 2 &&
            j.contains("type")  && j.at("type").is_string() &&
            j.contains("value") && j.at("value").is_string() )
    {
        const auto type  = j.at("type" ).get<std::string>();
        const auto value = j.at("value").get<std::string>();
        if(type == "string")
        {
            return toml::value(value);
        }
        else if(type == "int")
        {
            toml::detail::context<toml::type_config> ctx(toml::spec::default_version());
            auto loc = toml::detail::make_temporary_location(value);
            return toml::detail::parse_integer(loc, ctx).unwrap();
        }
        else if(type == "float")
        {
            toml::detail::context<toml::type_config> ctx(toml::spec::default_version());
            auto loc = toml::detail::make_temporary_location(value);
            if(auto f_r = toml::detail::parse_floating(loc, ctx))
            {
                return f_r.unwrap();
            }
            else // not conform TOML floating-point syntax.
            {
                // toml-test converts "inf" into "Inf"
                if(value == "Inf" || value == "+Inf")
                {
                    return toml::value(std::numeric_limits<double>::infinity());
                }
                else if(value == "-Inf")
                {
                    return toml::value(-std::numeric_limits<double>::infinity());
                }
                else // sometimes toml-test uses large int with type:float
                {
                    toml::floating_format_info fmt;
                    fmt.prec = value.size();
                    return toml::value(toml::detail::from_string<double>(value).unwrap(), fmt);
                }
            }
        }
        else
        {
            return toml::detail::literal_internal_impl(
                    toml::detail::make_temporary_location(value));
        }
    }
    else // table.
    {
        toml::value t(toml::table{});
        for(const auto& kv : j.items())
        {
            t[kv.key()] = convert_to_toml(kv.value());
        }
        return t;
    }
}

int main(int argc, char** argv)
{
    try
    {
        if(argc == 2)
        {
            const std::string fname(argv[1]);
            std::ifstream ifs(fname);
            const auto j = nlohmann::json::parse(ifs);
            const auto t = convert_to_toml(j);
            std::cout << toml::format(t) << std::endl;
            return 0;
        }
        else
        {
            std::vector<char> buf;
            std::cin.peek();

            while(!std::cin.eof())
            {
                buf.push_back(static_cast<char>(std::cin.get()));
                std::cin.peek();
            }
            std::string str(buf.begin(), buf.end());
            const auto j = nlohmann::json::parse(str);
            const auto t = convert_to_toml(j);
            std::cout << toml::format(t) << std::endl;
            return 0;
        }
    }
    catch(const std::exception& err)
    {
        std::cout << "what(): " << err.what() << std::endl;
        return 1;
    }
}

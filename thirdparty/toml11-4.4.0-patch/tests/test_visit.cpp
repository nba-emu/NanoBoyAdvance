#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/comments.hpp>
#include <toml11/types.hpp>
#include <toml11/value.hpp>
#include <toml11/visit.hpp>

template<typename Value>
struct printer
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

    std::string operator()(const boolean_type& x) const
    {
        if(x) {return "true";} else {return "false";}
    }
    std::string operator()(const integer_type& x) const
    {
        return std::to_string(x);
    }
    std::string operator()(const floating_type& x) const
    {
        return std::to_string(x);
    }
    std::string operator()(const string_type& x) const
    {
        return "\"" + x + "\"";
    }
    std::string operator()(const local_time_type& x) const
    {
        std::ostringstream oss;
        oss << x;
        return oss.str();
    }
    std::string operator()(const local_date_type& x) const
    {
        std::ostringstream oss;
        oss << x;
        return oss.str();
    }
    std::string operator()(const local_datetime_type& x) const
    {
        std::ostringstream oss;
        oss << x;
        return oss.str();
    }
    std::string operator()(const offset_datetime_type& x) const
    {
        std::ostringstream oss;
        oss << x;
        return oss.str();
    }
    std::string operator()(const array_type& a) const
    {
        std::string str;
        str += '[';
        for(const auto& e : a)
        {
            str += toml::visit(*this, e);
            str += ", ";
        }
        str += ']';
        return str;
    }

    std::string operator()(const table_type& t) const
    {
        std::string str;
        str += '{';
        for(const auto& e : t)
        {
            str += e.first;
            str += " = ";
            str += toml::visit(*this, e.second);
            str += ", ";
        }
        if( ! t.empty())
        {
            str.pop_back();
            str.pop_back();
        }
        str += '}';
        return str;
    }
};

struct type_config_ordered_map
{
    using comment_type  = toml::preserve_comments;

    using boolean_type  = bool;
    using integer_type  = std::int64_t;
    using floating_type = double;
    using string_type   = std::string;

    template<typename T>
    using array_type = std::vector<T>;
    template<typename K, typename T>
    using table_type = std::map<K, T>;

    static toml::result<integer_type, toml::error_info>
    parse_int(const std::string& str, const toml::source_location src, const std::uint8_t base)
    {
        assert(base == 10 || base == 16 || base == 8 || base == 2);
        return toml::read_int<integer_type>(str, src, base);
    }
    static toml::result<floating_type, toml::error_info>
    parse_float(const std::string& str, const toml::source_location src, const bool is_hex)
    {
        return toml::read_float<floating_type>(str, src, is_hex);
    }
};

TEST_CASE("testing toml::get with toml types")
{
    using value_type = toml::basic_value<type_config_ordered_map>;
    using array_type = typename value_type::array_type;
    using table_type = typename value_type::table_type;

    const printer<value_type> p;
    {
        value_type v(true);
        CHECK_EQ("true", toml::visit(p, v));
    }
    {
        value_type v(false);
        CHECK_EQ("false", toml::visit(p, v));
    }
    {
        value_type v(42);
        CHECK_EQ(std::to_string(42), toml::visit(p, v));
    }
    {
        value_type v(3.14);
        CHECK_EQ(std::to_string(3.14), toml::visit(p, v));
    }
    {
        value_type v("foo");
        CHECK_EQ("\"foo\"", toml::visit(p, v));
    }
    {
        value_type v(array_type{true, 42, 3.14, "foo"});
        CHECK_EQ("[true, 42, " + std::to_string(3.14) + ", \"foo\", ]", toml::visit(p, v));
    }
    {
        value_type v(table_type{{"foo", true}, {"bar", 42}});
        CHECK_EQ("{bar = 42, foo = true}", toml::visit(p, v));
    }
}


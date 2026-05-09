#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/parser.hpp>
#include <toml11/serializer.hpp>
#include <toml11/types.hpp>

TEST_CASE("testing fractional float")
{
    auto fmt = [](std::size_t prec) {
        toml::floating_format_info f;
        f.fmt = toml::floating_format::fixed;
        f.prec = prec;
        return f;
    };

    auto format_as = [](const double f, int w) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss << std::fixed << std::setprecision(w) << f;
        return oss.str();
    };

    CHECK_EQ(format_as( 1.0           ,  1), toml::format(toml::value( 1.0           , fmt( 1))));
    CHECK_EQ(format_as( 0.1           ,  1), toml::format(toml::value( 0.1           , fmt( 1))));
    CHECK_EQ(format_as( 0.001         ,  3), toml::format(toml::value( 0.001         , fmt( 3))));
    CHECK_EQ(format_as( 0.1           ,  3), toml::format(toml::value( 0.1           , fmt( 3))));
    CHECK_EQ(format_as( 3.14          ,  2), toml::format(toml::value( 3.14          , fmt( 2))));
    CHECK_EQ(format_as(-3.14          ,  2), toml::format(toml::value(-3.14          , fmt( 2))));
    CHECK_EQ(format_as( 3.141592653589, 12), toml::format(toml::value( 3.141592653589, fmt(12))));
    CHECK_EQ(format_as(-3.141592653589, 12), toml::format(toml::value(-3.141592653589, fmt(12))));
}

TEST_CASE("testing scientific float")
{
    auto fmt = [](std::size_t prec) {
        toml::floating_format_info f;
        f.fmt = toml::floating_format::scientific;
        f.prec = prec;
        return f;
    };

    auto format_as = [](const double f, int w) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss << std::scientific << std::setprecision(w) << f;
        return oss.str();
    };

    CHECK_EQ(format_as( 1.0           ,  1), toml::format(toml::value( 1.0           , fmt( 1))));
    CHECK_EQ(format_as( 0.1           ,  1), toml::format(toml::value( 0.1           , fmt( 1))));
    CHECK_EQ(format_as( 0.001         ,  3), toml::format(toml::value( 0.001         , fmt( 3))));
    CHECK_EQ(format_as( 0.1           ,  3), toml::format(toml::value( 0.1           , fmt( 3))));
    CHECK_EQ(format_as( 3.14          ,  2), toml::format(toml::value( 3.14          , fmt( 2))));
    CHECK_EQ(format_as(-3.14          ,  2), toml::format(toml::value(-3.14          , fmt( 2))));
    CHECK_EQ(format_as( 3.141592653589, 12), toml::format(toml::value( 3.141592653589, fmt(12))));
    CHECK_EQ(format_as(-3.141592653589, 12), toml::format(toml::value(-3.141592653589, fmt(12))));
}

TEST_CASE("testing hex float")
{
    toml::spec s = toml::spec::v(1,0,0);
    s.ext_hex_float = true;

    toml::floating_format_info fmt;
    fmt.fmt = toml::floating_format::hex;

    auto format_as = [](const double f) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss << std::hexfloat << f;
        return oss.str();
    };

    CHECK_EQ(format_as( 1.0           ), toml::format(toml::value( 1.0           , fmt), s));
    CHECK_EQ(format_as( 0.1           ), toml::format(toml::value( 0.1           , fmt), s));
    CHECK_EQ(format_as( 0.001         ), toml::format(toml::value( 0.001         , fmt), s));
    CHECK_EQ(format_as( 0.1           ), toml::format(toml::value( 0.1           , fmt), s));
    CHECK_EQ(format_as( 3.14          ), toml::format(toml::value( 3.14          , fmt), s));
    CHECK_EQ(format_as(-3.14          ), toml::format(toml::value(-3.14          , fmt), s));
    CHECK_EQ(format_as( 3.141592653589), toml::format(toml::value( 3.141592653589, fmt), s));
    CHECK_EQ(format_as(-3.141592653589), toml::format(toml::value(-3.141592653589, fmt), s));
}

TEST_CASE("testing suffix + fractional")
{
    toml::spec s = toml::spec::v(1,0,0);
    s.ext_hex_float = true;
    s.ext_num_suffix = true;

    auto fmt = [](std::string sfx) {
        toml::floating_format_info f;
        f.fmt = toml::floating_format::fixed;
        f.suffix = sfx;
        return f;
    };

    auto format_as = [](const double f, std::string sfx) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss << std::fixed << f << "_" << sfx;
        return oss.str();
    };

    CHECK_EQ(format_as( 1.0           , "m"  ), toml::format(toml::value( 1.0           , fmt("m"  )), s));
    CHECK_EQ(format_as( 0.1           , "m"  ), toml::format(toml::value( 0.1           , fmt("m"  )), s));
    CHECK_EQ(format_as( 0.001         , "mm" ), toml::format(toml::value( 0.001         , fmt("mm" )), s));
    CHECK_EQ(format_as( 0.1           , "mm" ), toml::format(toml::value( 0.1           , fmt("mm" )), s));
    CHECK_EQ(format_as( 3.14          , "rad"), toml::format(toml::value( 3.14          , fmt("rad")), s));
    CHECK_EQ(format_as(-3.14          , "rad"), toml::format(toml::value(-3.14          , fmt("rad")), s));
    CHECK_EQ(format_as( 3.141592653589, "rad"), toml::format(toml::value( 3.141592653589, fmt("rad")), s));
    CHECK_EQ(format_as(-3.141592653589, "rad"), toml::format(toml::value(-3.141592653589, fmt("rad")), s));
}

TEST_CASE("testing suffix + scientific")
{
    toml::spec s = toml::spec::v(1,0,0);
    s.ext_hex_float = true;
    s.ext_num_suffix = true;

    auto fmt = [](std::string sfx) {
        toml::floating_format_info f;
        f.fmt = toml::floating_format::scientific;
        f.suffix = sfx;
        return f;
    };

    auto format_as = [](const double f, std::string sfx) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss << std::scientific << f << "_" << sfx;
        return oss.str();
    };

    CHECK_EQ(format_as( 1.0           , "m"  ), toml::format(toml::value( 1.0           , fmt("m"  )), s));
    CHECK_EQ(format_as( 0.1           , "m"  ), toml::format(toml::value( 0.1           , fmt("m"  )), s));
    CHECK_EQ(format_as( 0.001         , "mm" ), toml::format(toml::value( 0.001         , fmt("mm" )), s));
    CHECK_EQ(format_as( 0.1           , "mm" ), toml::format(toml::value( 0.1           , fmt("mm" )), s));
    CHECK_EQ(format_as( 3.14          , "rad"), toml::format(toml::value( 3.14          , fmt("rad")), s));
    CHECK_EQ(format_as(-3.14          , "rad"), toml::format(toml::value(-3.14          , fmt("rad")), s));
    CHECK_EQ(format_as( 3.141592653589, "rad"), toml::format(toml::value( 3.141592653589, fmt("rad")), s));
    CHECK_EQ(format_as(-3.141592653589, "rad"), toml::format(toml::value(-3.141592653589, fmt("rad")), s));
}

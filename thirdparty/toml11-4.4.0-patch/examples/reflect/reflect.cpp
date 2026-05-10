#include <iostream>

#include <reflect>
#include "reflect.hpp"

struct Hoge
{
    int foo;
    double bar;
    std::string baz;
};
TOML11_REFLECT(Hoge)

int main()
{
    toml::value v(toml::table{
            {"foo", 42},
            {"bar", 3.14},
            {"baz", "fuga"},
        });

    const Hoge h = toml::get<Hoge>(v);

    std::cout << "Hoge.foo = " << h.foo << std::endl;
    std::cout << "Hoge.bar = " << h.bar << std::endl;
    std::cout << "Hoge.baz = " << h.baz << std::endl;

    Hoge h2;
    h2.foo = 6 * 9;
    h2.bar = 2.718;
    h2.baz = "piyo";

    toml::value v2(h2);

    std::cout << toml::format(v2);

    return 0;
}

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml.hpp>
#include "utility.hpp"

#if defined(TOML11_HAS_OPTIONAL)
#include <optional>
#endif

namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
struct bar
{
    int a;
    std::string b;

    void from_toml(const toml::value& v)
    {
        this->a = toml::find<int>(v, "a");
        this->b = toml::find<std::string>(v, "b");
        return ;
    }

    toml::table into_toml() const
    {
        return toml::table{{"a", this->a}, {"b", this->b}};
    }
};

struct baz
{
    int a;
    std::string b;
};
struct qux
{
    int a;
    std::string b;
};

struct foobar
{
    // via constructor
    explicit foobar(const toml::value& v)
        : a(toml::find<int>(v, "a")), b(toml::find<std::string>(v, "b"))
    {}
    int a;
    std::string b;
};

struct corge
{
    int a;
    std::string b;

    void from_toml(const toml::value& v)
    {
        this->a = toml::find<int>(v, "a");
        this->b = toml::find<std::string>(v, "b");
        return ;
    }

    template <typename TC>
    toml::basic_value<TC> into_toml() const
    {
        return toml::basic_value<TC>(typename toml::basic_value<TC>::table_type{{"a", this->a}, {"b", this->b}});
    }
};
} // extlib

namespace toml
{
template<>
struct from<extlib::foo>
{
    static extlib::foo from_toml(const toml::value& v)
    {
        return extlib::foo{toml::find<int>(v, "a"), toml::find<std::string>(v, "b")};
    }
};

template<>
struct into<extlib::foo>
{
    template<typename TC>
    static toml::basic_value<TC> into_toml(const extlib::foo& f)
    {
        return toml::basic_value<TC>(typename toml::basic_value<TC>::table_type{{"a", f.a}, {"b", f.b}});
    }
};

template<>
struct from<extlib::baz>
{
    static extlib::baz from_toml(const toml::value& v)
    {
        return extlib::baz{toml::find<int>(v, "a"), toml::find<std::string>(v, "b")};
    }
};

template<>
struct into<extlib::qux>
{
    template<typename TC>
    static toml::basic_value<TC> into_toml(const extlib::qux& f)
    {
        return toml::basic_value<TC>(typename toml::basic_value<TC>::table_type{{"a", f.a}, {"b", f.b}});
    }
};
} // toml

// ---------------------------------------------------------------------------

namespace extlib2
{
struct foo
{
    int a;
    std::string b;
};
struct bar
{
    int a;
    std::string b;

    template<typename TC>
    void from_toml(const toml::basic_value<TC>& v)
    {
        this->a = toml::find<int>(v, "a");
        this->b = toml::find<std::string>(v, "b");
        return ;
    }

    toml::ordered_table into_toml() const
    {
        return toml::ordered_table{{"a", this->a}, {"b", this->b}};
    }
};
struct baz
{
    int a;
    std::string b;
};
struct qux
{
    int a;
    std::string b;
};

struct foobar
{
    template<typename TC>
    explicit foobar(const toml::basic_value<TC>& v)
        : a(toml::find<int>(v, "a")), b(toml::find<std::string>(v, "b"))
    {}
    int a;
    std::string b;
};

} // extlib2

namespace toml
{
template<>
struct from<extlib2::foo>
{
    template<typename TC>
    static extlib2::foo from_toml(const toml::basic_value<TC>& v)
    {
        return extlib2::foo{toml::find<int>(v, "a"), toml::find<std::string>(v, "b")};
    }
};

template<>
struct into<extlib2::foo>
{
    template<typename TC>
    static toml::basic_value<TC> into_toml(const extlib2::foo& f)
    {
        return toml::basic_value<TC>(typename toml::basic_value<TC>::table_type{{"a", f.a}, {"b", f.b}});
    }
};

template<>
struct from<extlib2::baz>
{
    template<typename TC>
    static extlib2::baz from_toml(const toml::basic_value<TC>& v)
    {
        return extlib2::baz{toml::find<int>(v, "a"), toml::find<std::string>(v, "b")};
    }
};

template<>
struct into<extlib2::qux>
{
    template<typename TC>
    static toml::basic_value<TC>
    into_toml(const extlib2::qux& f)
    {
        return toml::basic_value<TC>{
            {"a", f.a}, {"b", f.b}
        };
    }
};
} // toml

// ---------------------------------------------------------------------------

TEST_CASE("test_conversion_by_member_methods")
{
    {
        const toml::value v(toml::table{{"a", 42}, {"b", "baz"}});

        const auto foo = toml::get<extlib::foo>(v);
        CHECK_EQ(foo.a, 42);
        CHECK_EQ(foo.b, "baz");

        const toml::value v2(foo);

        CHECK_EQ(v, v2);
    }

    {
        const toml::value v(toml::table{{"a", 42}, {"b", "baz"}});

        const auto foo = toml::get<extlib2::foo>(v);
        CHECK(foo.a == 42);
        CHECK(foo.b == "baz");

        const toml::value v2(foo);
        CHECK(v == v2);
    }


    {
        const toml::value v(toml::table{{"a", 42}, {"b", "baz"}});

        const auto corge = toml::get<extlib::corge>(v);
        CHECK_EQ(corge.a, 42);
        CHECK_EQ(corge.b, "baz");

        const toml::value v2(corge);

        CHECK_EQ(v, v2);
    }

    {
        const toml::ordered_value v(toml::ordered_table{{"a", 42}, {"b", "baz"}});

        const auto foo = toml::get<extlib2::foo>(v);
        CHECK(foo.a == 42);
        CHECK(foo.b == "baz");

        const toml::ordered_value v2(foo);

        CHECK(v == v2);
    }
}

TEST_CASE("test_conversion_by_specialization")
{
    {
        const toml::value v(toml::table{{"a", 42}, {"b", "baz"}});

        const auto bar = toml::get<extlib::bar>(v);
        CHECK(bar.a == 42);
        CHECK(bar.b == "baz");

        const toml::value v2(bar);

        CHECK(v == v2);
    }
    {
        const toml::value v(toml::table{{"a", 42}, {"b", "baz"}});

        const auto bar = toml::get<extlib2::bar>(v);
        CHECK(bar.a == 42);
        CHECK(bar.b == "baz");

        const toml::value v2(bar);

        CHECK(v == v2);
    }
    {
        const toml::ordered_value v(toml::ordered_table{{"a", 42}, {"b", "baz"}});

        const auto bar = toml::get<extlib2::bar>(v);
        CHECK(bar.a == 42);
        CHECK(bar.b == "baz");

        const toml::ordered_value v2(bar);

        CHECK(v == v2);
    }
}

TEST_CASE("test_conversion_one_way")
{
    {
        const toml::value v(toml::table{{"a", 42}, {"b", "baz"}});

        const auto baz = toml::get<extlib::baz>(v);
        CHECK(baz.a == 42);
        CHECK(baz.b == "baz");
    }
    {
        const extlib::qux q{42, "qux"};
        const toml::value v(q);

        CHECK(toml::find<int>(v, "a")         == 42);
        CHECK(toml::find<std::string>(v, "b") == "qux");
    }

    {
        const toml::ordered_value v(toml::ordered_table{
            {"a", 42}, {"b", "baz"}
        });

        const auto baz = toml::get<extlib2::baz>(v);
        CHECK(baz.a == 42);
        CHECK(baz.b == "baz");
    }
    {
        const extlib::qux q{42, "qux"};
        const toml::ordered_value v(q);

        CHECK(toml::find<int>(v, "a")         == 42);
        CHECK(toml::find<std::string>(v, "b") == "qux");
    }
}

TEST_CASE("test_conversion_via_constructor")
{
    {
        const toml::value v(toml::table{{"a", 42}, {"b", "foobar"}});

        const auto foobar = toml::get<extlib::foobar>(v);
        CHECK(foobar.a == 42);
        CHECK(foobar.b == "foobar");
    }

    {
        const toml::ordered_value v(toml::ordered_table{
            {"a", 42}, {"b", "foobar"}
        });

        const auto foobar = toml::get<extlib2::foobar>(v);
        CHECK(foobar.a == 42);
        CHECK(foobar.b == "foobar");
    }
}

TEST_CASE("test_recursive_conversion")
{
    {
        const toml::value v(toml::array{
            toml::table{{"a", 42}, {"b", "baz"}},
            toml::table{{"a", 43}, {"b", "qux"}},
            toml::table{{"a", 44}, {"b", "quux"}},
            toml::table{{"a", 45}, {"b", "foobar"}},
        });

        const auto foos = toml::get<std::vector<extlib::foo>>(v);
        CHECK(foos.size()  == 4ul);
        CHECK(foos.at(0).a == 42);
        CHECK(foos.at(1).a == 43);
        CHECK(foos.at(2).a == 44);
        CHECK(foos.at(3).a == 45);

        CHECK(foos.at(0).b == "baz");
        CHECK(foos.at(1).b == "qux");
        CHECK(foos.at(2).b == "quux");
        CHECK(foos.at(3).b == "foobar");

        const auto bars = toml::get<std::vector<extlib::bar>>(v);
        CHECK(bars.size()  == 4ul);
        CHECK(bars.at(0).a == 42);
        CHECK(bars.at(1).a == 43);
        CHECK(bars.at(2).a == 44);
        CHECK(bars.at(3).a == 45);

        CHECK(bars.at(0).b == "baz");
        CHECK(bars.at(1).b == "qux");
        CHECK(bars.at(2).b == "quux");
        CHECK(bars.at(3).b == "foobar");
    }
    {
        const toml::value v(toml::array{
                toml::table{{"a", 42}, {"b", "baz"}},
                toml::table{{"a", 43}, {"b", "qux"}},
                toml::table{{"a", 44}, {"b", "quux"}},
                toml::table{{"a", 45}, {"b", "foobar"}},
            });

        const auto foos = toml::get<std::vector<extlib2::foo>>(v);
        CHECK(foos.size()  == 4ul);
        CHECK(foos.at(0).a == 42);
        CHECK(foos.at(1).a == 43);
        CHECK(foos.at(2).a == 44);
        CHECK(foos.at(3).a == 45);

        CHECK(foos.at(0).b == "baz");
        CHECK(foos.at(1).b == "qux");
        CHECK(foos.at(2).b == "quux");
        CHECK(foos.at(3).b == "foobar");

        const auto bars = toml::get<std::vector<extlib2::bar>>(v);
        CHECK(bars.size()  == 4ul);
        CHECK(bars.at(0).a == 42);
        CHECK(bars.at(1).a == 43);
        CHECK(bars.at(2).a == 44);
        CHECK(bars.at(3).a == 45);

        CHECK(bars.at(0).b == "baz");
        CHECK(bars.at(1).b == "qux");
        CHECK(bars.at(2).b == "quux");
        CHECK(bars.at(3).b == "foobar");
    }

    {
        const toml::ordered_value v(toml::ordered_array{
                toml::ordered_table{{"a", 42}, {"b", "baz"}},
                toml::ordered_table{{"a", 43}, {"b", "qux"}},
                toml::ordered_table{{"a", 44}, {"b", "quux"}},
                toml::ordered_table{{"a", 45}, {"b", "foobar"}}
            });

        const auto foos = toml::get<std::vector<extlib2::foo>>(v);
        CHECK(foos.size()  == 4ul);
        CHECK(foos.at(0).a == 42);
        CHECK(foos.at(1).a == 43);
        CHECK(foos.at(2).a == 44);
        CHECK(foos.at(3).a == 45);

        CHECK(foos.at(0).b == "baz");
        CHECK(foos.at(1).b == "qux");
        CHECK(foos.at(2).b == "quux");
        CHECK(foos.at(3).b == "foobar");

        const auto bars = toml::get<std::vector<extlib2::bar>>(v);
        CHECK(bars.size()  == 4ul);
        CHECK(bars.at(0).a == 42);
        CHECK(bars.at(1).a == 43);
        CHECK(bars.at(2).a == 44);
        CHECK(bars.at(3).a == 45);

        CHECK(bars.at(0).b == "baz");
        CHECK(bars.at(1).b == "qux");
        CHECK(bars.at(2).b == "quux");
        CHECK(bars.at(3).b == "foobar");
    }

    // via constructor
    {
        const toml::value v(toml::array{
                toml::table{{"a", 42}, {"b", "baz"}},
                toml::table{{"a", 43}, {"b", "qux"}},
                toml::table{{"a", 44}, {"b", "quux"}},
                toml::table{{"a", 45}, {"b", "foobar"}}
            });

        {
            const auto foobars = toml::get<std::vector<extlib::foobar>>(v);
            CHECK(foobars.size()  == 4ul);
            CHECK(foobars.at(0).a == 42);
            CHECK(foobars.at(1).a == 43);
            CHECK(foobars.at(2).a == 44);
            CHECK(foobars.at(3).a == 45);

            CHECK(foobars.at(0).b == "baz");
            CHECK(foobars.at(1).b == "qux");
            CHECK(foobars.at(2).b == "quux");
            CHECK(foobars.at(3).b == "foobar");
        }
        {
            const auto foobars = toml::get<std::vector<extlib2::foobar>>(v);
            CHECK(foobars.size()  == 4ul);
            CHECK(foobars.at(0).a == 42);
            CHECK(foobars.at(1).a == 43);
            CHECK(foobars.at(2).a == 44);
            CHECK(foobars.at(3).a == 45);

            CHECK(foobars.at(0).b == "baz");
            CHECK(foobars.at(1).b == "qux");
            CHECK(foobars.at(2).b == "quux");
            CHECK(foobars.at(3).b == "foobar");
        }
    }
    {
        const toml::ordered_value v(toml::ordered_array{
                toml::ordered_table{{"a", 42}, {"b", "baz"}},
                toml::ordered_table{{"a", 43}, {"b", "qux"}},
                toml::ordered_table{{"a", 44}, {"b", "quux"}},
                toml::ordered_table{{"a", 45}, {"b", "foobar"}}
            });

        const auto foobars = toml::get<std::vector<extlib2::foobar>>(v);
        CHECK(foobars.size()  == 4ul);
        CHECK(foobars.at(0).a == 42);
        CHECK(foobars.at(1).a == 43);
        CHECK(foobars.at(2).a == 44);
        CHECK(foobars.at(3).a == 45);

        CHECK(foobars.at(0).b == "baz");
        CHECK(foobars.at(1).b == "qux");
        CHECK(foobars.at(2).b == "quux");
        CHECK(foobars.at(3).b == "foobar");
    }

    // via constructor
    {
        const toml::value v(toml::table{
                {"0", toml::table{{"a", 42}, {"b", "baz"}}},
                {"1", toml::table{{"a", 43}, {"b", "qux"}}},
                {"2", toml::table{{"a", 44}, {"b", "quux"}}},
                {"3", toml::table{{"a", 45}, {"b", "foobar"}}}
            });

        {
            const auto foobars = toml::get<std::map<std::string, extlib::foobar>>(v);
            CHECK(foobars.size()  == 4ul);
            CHECK(foobars.at("0").a == 42);
            CHECK(foobars.at("1").a == 43);
            CHECK(foobars.at("2").a == 44);
            CHECK(foobars.at("3").a == 45);

            CHECK(foobars.at("0").b == "baz");
            CHECK(foobars.at("1").b == "qux");
            CHECK(foobars.at("2").b == "quux");
            CHECK(foobars.at("3").b == "foobar");
        }
        {
            const auto foobars = toml::get<std::map<std::string, extlib2::foobar>>(v);
            CHECK(foobars.size()  == 4ul);
            CHECK(foobars.at("0").a == 42);
            CHECK(foobars.at("1").a == 43);
            CHECK(foobars.at("2").a == 44);
            CHECK(foobars.at("3").a == 45);

            CHECK(foobars.at("0").b == "baz");
            CHECK(foobars.at("1").b == "qux");
            CHECK(foobars.at("2").b == "quux");
            CHECK(foobars.at("3").b == "foobar");
        }
    }
    {
        const toml::ordered_value v(toml::ordered_table{
                {"0", toml::ordered_table{{"a", 42}, {"b", "baz"}}},
                {"1", toml::ordered_table{{"a", 43}, {"b", "qux"}}},
                {"2", toml::ordered_table{{"a", 44}, {"b", "quux"}}},
                {"3", toml::ordered_table{{"a", 45}, {"b", "foobar"}}}
            });

        const auto foobars = toml::get<std::map<std::string, extlib::foobar>>(v);
        CHECK(foobars.size()  == 4ul);
        CHECK(foobars.at("0").a == 42);
        CHECK(foobars.at("1").a == 43);
        CHECK(foobars.at("2").a == 44);
        CHECK(foobars.at("3").a == 45);

        CHECK(foobars.at("0").b == "baz");
        CHECK(foobars.at("1").b == "qux");
        CHECK(foobars.at("2").b == "quux");
        CHECK(foobars.at("3").b == "foobar");
    }

}

// ===========================================================================

#ifndef TOML11_WITHOUT_DEFINE_NON_INTRUSIVE

namespace extlib3
{
struct foo
{
    int a;
    std::string b;
};
struct bar
{
    int         a;
    std::string b;
    foo         f;
};

} // extlib3

TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(extlib3::foo, a, b)
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(extlib3::bar, a, b, f)

TEST_CASE("test_conversion_via_macro")
{
    {
        const toml::value v(toml::table{{"a", 42}, {"b", "baz"}});

        const auto foo = toml::get<extlib3::foo>(v);
        CHECK(foo.a == 42);
        CHECK(foo.b == "baz");

        const toml::value v2(foo);
        CHECK(v2 == v);
    }
    {
        const toml::ordered_value v(toml::ordered_table{
            {"a", 42}, {"b", "baz"}
        });

        const auto foo = toml::get<extlib3::foo>(v);
        CHECK(foo.a == 42);
        CHECK(foo.b == "baz");

        const toml::ordered_value v2(foo);
        CHECK(v2 == v);
    }

    // -----------------------------------------------------------------------

    {
        const toml::value v(toml::table{
            {"a", 42},
            {"b", "bar.b"},
            {"f", toml::table{{"a", 42}, {"b", "foo.b"}}}
        });

        const auto bar = toml::get<extlib3::bar>(v);
        CHECK(bar.a == 42);
        CHECK(bar.b == "bar.b");
        CHECK(bar.f.a == 42);
        CHECK(bar.f.b == "foo.b");

        const toml::value v2(bar);
        CHECK(v2 == v);
    }
    {
        const toml::ordered_value v(toml::ordered_table{
            {"a", 42},
            {"b", "bar.b"},
            {"f", toml::ordered_table{{"a", 42}, {"b", "foo.b"}}}
        });

        const auto bar = toml::get<extlib3::bar>(v);
        CHECK(bar.a == 42);
        CHECK(bar.b == "bar.b");
        CHECK(bar.f.a == 42);
        CHECK(bar.f.b == "foo.b");

        const toml::ordered_value v2(bar);
        CHECK(v2 == v);
    }
}

#if defined(TOML11_HAS_OPTIONAL)
namespace extlib4
{
struct foo
{
    std::optional<int>         a;
    std::optional<std::string> b;
};
struct bar
{
    std::optional<int>         a;
    std::optional<std::string> b;
    std::optional<foo>         f;
};

} // extlib4

TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(extlib4::foo, a, b)
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(extlib4::bar, a, b, f)

TEST_CASE("test_optional_conversion_via_macro")
{
    {
        const toml::value v(toml::table{{"a", 42}});

        const auto foo = toml::get<extlib4::foo>(v);
        CHECK(foo.a.value() == 42);
        CHECK(foo.b == std::nullopt);

        const toml::value v2(foo);
        CHECK(v2 == v);
    }
    {
        const toml::ordered_value v(toml::ordered_table{
            {"b", "baz"}
        });

        const auto foo = toml::get<extlib4::foo>(v);
        CHECK(foo.a == std::nullopt);
        CHECK(foo.b.value() == "baz");

        const toml::ordered_value v2(foo);
        CHECK(v2 == v);
    }

    // -----------------------------------------------------------------------

    {
        const toml::value v(toml::table{
            {"b", "bar.b"},
            {"f", toml::table{{"a", 42}}}
        });

        const auto bar = toml::get<extlib4::bar>(v);
        CHECK(bar.a == std::nullopt);
        CHECK(bar.b.value() == "bar.b");
        CHECK(bar.f.value().a.value() == 42);
        CHECK(bar.f.value().b == std::nullopt);

        const toml::value v2(bar);
        CHECK(v2 == v);
    }
    {
        const toml::ordered_value v(toml::ordered_table{
            {"a", 42},
            {"f", toml::ordered_table{{"b", "foo.b"}}}
        });

        const auto bar = toml::get<extlib4::bar>(v);
        CHECK(bar.a.value() == 42);
        CHECK(bar.b == std::nullopt);
        CHECK(bar.f.value().a == std::nullopt);
        CHECK(bar.f.value().b.value() == "foo.b");

        const toml::ordered_value v2(bar);
        CHECK(v2 == v);
    }
}
#endif // TOML11_HAS_OPTIONAL
#endif // TOML11_WITHOUT_DEFINE_NON_INTRUSIVE

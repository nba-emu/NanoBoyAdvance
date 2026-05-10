#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/scanner.hpp>

TEST_CASE("testing scanner: character")
{
    auto loc = toml::detail::make_temporary_location("\t \tA");

    toml::detail::character tab('\t');
    toml::detail::character space(' ');

    CHECK_UNARY( tab  .scan(loc).is_ok()); // loc += 1
    CHECK_UNARY(!tab  .scan(loc).is_ok());
    CHECK_UNARY( space.scan(loc).is_ok());
    CHECK_UNARY( tab  .scan(loc).is_ok());
    CHECK_UNARY(!tab  .scan(loc).is_ok());
    CHECK_UNARY(!space.scan(loc).is_ok());
}

TEST_CASE("testing scanner: character_either")
{
    auto loc = toml::detail::make_temporary_location("\t \t01\0");

    toml::detail::character_either wschar("\t ");
    toml::detail::character_either digit01("01");

    CHECK_UNARY( wschar .scan(loc).is_ok());
    CHECK_UNARY( wschar .scan(loc).is_ok());
    CHECK_UNARY( wschar .scan(loc).is_ok());
    CHECK_UNARY(!wschar .scan(loc).is_ok());
    CHECK_UNARY( digit01.scan(loc).is_ok());
    CHECK_UNARY( digit01.scan(loc).is_ok());
    CHECK_UNARY(!digit01.scan(loc).is_ok());
    CHECK_UNARY(!wschar .scan(loc).is_ok());
}

TEST_CASE("testing scanner: character_in_range")
{
    auto loc = toml::detail::make_temporary_location("abcdz12349");

    toml::detail::character_in_range alpha('a', 'z');
    toml::detail::character_in_range digit('0', '9');

    CHECK_UNARY( alpha.scan(loc).is_ok());
    CHECK_UNARY( alpha.scan(loc).is_ok());
    CHECK_UNARY( alpha.scan(loc).is_ok());
    CHECK_UNARY( alpha.scan(loc).is_ok());
    CHECK_UNARY( alpha.scan(loc).is_ok());
    CHECK_UNARY(!alpha.scan(loc).is_ok());
    CHECK_UNARY( digit.scan(loc).is_ok());
    CHECK_UNARY( digit.scan(loc).is_ok());
    CHECK_UNARY( digit.scan(loc).is_ok());
    CHECK_UNARY( digit.scan(loc).is_ok());
    CHECK_UNARY( digit.scan(loc).is_ok());
    CHECK_UNARY(!digit.scan(loc).is_ok());
}

TEST_CASE("testing scanner: literal")
{
    auto loc = toml::detail::make_temporary_location("abcdz12349");

    toml::detail::literal abcd("abcd");
    toml::detail::literal z123("z123");

    CHECK_UNARY( abcd.scan(loc).is_ok());
    CHECK_UNARY(!abcd.scan(loc).is_ok());
    CHECK_UNARY( z123.scan(loc).is_ok());
    CHECK_UNARY(!z123.scan(loc).is_ok());
    CHECK_UNARY(!abcd.scan(loc).is_ok());
}

TEST_CASE("testing scanner: sequence")
{
    auto loc = toml::detail::make_temporary_location("abcdz12349");

    toml::detail::literal abcd("abcd");
    toml::detail::literal z123("z123");

    toml::detail::sequence seq(abcd, z123);

    CHECK_UNARY( seq.scan(loc).is_ok());
    CHECK_EQ( loc.current(), toml::detail::location::char_type('4'));

    auto loc2 = toml::detail::make_temporary_location("abcde12349");

    CHECK_UNARY(!seq.scan(loc2).is_ok());
}

TEST_CASE("testing scanner: either")
{
    auto loc = toml::detail::make_temporary_location("abcdz123");

    toml::detail::literal abcd("abcd");
    toml::detail::literal z123("z123");

    toml::detail::either e(abcd, z123);

    CHECK_UNARY( e.scan(loc).is_ok());
    CHECK_EQ( loc.current(), toml::detail::location::char_type('z'));

    CHECK_UNARY( e.scan(loc).is_ok());
    CHECK_UNARY( loc.eof() );
}

TEST_CASE("testing scanner: repeat_exact")
{
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");
        toml::detail::repeat_exact r(2, toml::detail::literal("foo"));

        CHECK_UNARY( r.scan(loc).is_ok());
        CHECK_EQ( loc.current(), toml::detail::location::char_type('b'));
    }

    {
        auto loc = toml::detail::make_temporary_location("foofoobar");

        toml::detail::repeat_exact r(3, toml::detail::literal("foo"));

        CHECK_UNARY( ! r.scan(loc).is_ok());
    }
}

TEST_CASE("testing scanner: repeat_at_least")
{
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");
        toml::detail::repeat_at_least r(1, toml::detail::literal("foo"));

        CHECK_UNARY( r.scan(loc).is_ok());
        CHECK_EQ( loc.current(), toml::detail::location::char_type('b'));
    }
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");
        toml::detail::repeat_at_least r(2, toml::detail::literal("foo"));

        CHECK_UNARY( r.scan(loc).is_ok());
        CHECK_EQ( loc.current(), toml::detail::location::char_type('b'));
    }
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");

        toml::detail::repeat_at_least r(3, toml::detail::literal("foo"));

        CHECK_UNARY( ! r.scan(loc).is_ok());
    }
}

TEST_CASE("testing scanner: maybe")
{
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");
        toml::detail::maybe r(toml::detail::literal("foo"));

        CHECK_UNARY( r.scan(loc).is_ok());
        CHECK_EQ( loc.current(), toml::detail::location::char_type('f'));
        CHECK_EQ( loc.get_location(), 3);
    }
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");
        toml::detail::maybe r(toml::detail::literal("bar"));

        CHECK_UNARY( r.scan(loc).is_ok());
        CHECK_EQ( loc.current(), toml::detail::location::char_type('f'));
        CHECK_EQ( loc.get_location(), 0);
    }
}

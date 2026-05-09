#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/types.hpp>

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct dummy_type{};

template<typename T>
struct dummy_container
{
    typedef T                 value_type;
    typedef value_type*       pointer;
    typedef value_type&       reference;
    typedef value_type const* const_pointer;
    typedef value_type const& const_reference;
    typedef pointer           iterator;
    typedef const_pointer     const_iterator;
};

TEST_CASE("testing has_xxx traits")
{
    CHECK_UNARY(toml::detail::has_iterator<std::list<dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_iterator<std::forward_list<dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_iterator<std::deque<dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_iterator<std::vector<dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_iterator<std::set<dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_iterator<std::unordered_set<std::string>>::value);
    CHECK_UNARY(toml::detail::has_iterator<std::array<dummy_type, 10>>::value);
    CHECK_UNARY(toml::detail::has_iterator<std::map<std::string, dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_iterator<std::unordered_map<std::string, dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_iterator<dummy_container<dummy_type>>::value);

    CHECK_UNARY(toml::detail::has_value_type<std::list<dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_value_type<std::forward_list<dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_value_type<std::deque<dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_value_type<std::vector<dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_value_type<std::array<dummy_type, 10>>::value);
    CHECK_UNARY(toml::detail::has_value_type<std::set<dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_value_type<std::unordered_set<std::string>>::value);
    CHECK_UNARY(toml::detail::has_value_type<std::map<std::string, dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_value_type<std::unordered_map<std::string, dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_value_type<dummy_container<dummy_type>>::value);

    CHECK_UNARY(toml::detail::has_key_type<std::map<std::string, dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_key_type<std::unordered_map<std::string, dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_mapped_type<std::map<std::string, dummy_type>>::value);
    CHECK_UNARY(toml::detail::has_mapped_type<std::unordered_map<std::string, dummy_type>>::value);
}

TEST_CASE("testing is_xxx traits")
{
    CHECK_UNARY(toml::detail::is_container<std::list<dummy_type>>::value);
    CHECK_UNARY(toml::detail::is_container<std::forward_list<dummy_type>>::value);
    CHECK_UNARY(toml::detail::is_container<std::deque<dummy_type>>::value);
    CHECK_UNARY(toml::detail::is_container<std::vector<dummy_type>>::value);
    CHECK_UNARY(toml::detail::is_container<std::array<dummy_type, 10>>::value);
    CHECK_UNARY(toml::detail::is_container<std::set<dummy_type>>::value);
    CHECK_UNARY(toml::detail::is_container<std::unordered_set<std::string>>::value);
    CHECK_UNARY(toml::detail::is_container<dummy_container<dummy_type>>::value);

    CHECK_UNARY_FALSE(toml::detail::is_container<std::map<std::string, dummy_type>>::value);
    CHECK_UNARY_FALSE(toml::detail::is_container<std::unordered_map<std::string, dummy_type>>::value);

    CHECK_UNARY(toml::detail::is_map<std::map<std::string, dummy_type>>::value);
    CHECK_UNARY(toml::detail::is_map<std::unordered_map<std::string, dummy_type>>::value);
}

TEST_CASE("testing make_index_sequence")
{
    static_assert(std::is_same<toml::cxx::make_index_sequence<0>, toml::cxx::index_sequence<>>::value, "");
    static_assert(std::is_same<toml::cxx::make_index_sequence<1>, toml::cxx::index_sequence<0>>::value, "");
    static_assert(std::is_same<toml::cxx::make_index_sequence<2>, toml::cxx::index_sequence<0,1>>::value, "");
    static_assert(std::is_same<toml::cxx::make_index_sequence<3>, toml::cxx::index_sequence<0,1,2>>::value, "");
    static_assert(std::is_same<toml::cxx::make_index_sequence<4>, toml::cxx::index_sequence<0,1,2,3>>::value, "");
    static_assert(std::is_same<toml::cxx::make_index_sequence<5>, toml::cxx::index_sequence<0,1,2,3,4>>::value, "");
    static_assert(std::is_same<toml::cxx::make_index_sequence<6>, toml::cxx::index_sequence<0,1,2,3,4,5>>::value, "");
    static_assert(std::is_same<toml::cxx::make_index_sequence<7>, toml::cxx::index_sequence<0,1,2,3,4,5,6>>::value, "");
    static_assert(std::is_same<toml::cxx::make_index_sequence<8>, toml::cxx::index_sequence<0,1,2,3,4,5,6,7>>::value, "");
}

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/syntax.hpp>

TEST_CASE("testing comment")
{
    const auto scanner = toml::detail::syntax::comment(toml::spec::v(1,0,0));

    test_scan_success(scanner, "#   hoge", "#   hoge");
    test_scan_success(scanner, "#   \n",   "#   ");
    test_scan_success(scanner, "#   \r\n", "#   ");
    test_scan_success(scanner, "# # \n",   "# # ");
}

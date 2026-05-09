#include <toml11/value.hpp>
#include <toml11/types.hpp>

#if ! defined(TOML11_COMPILE_SOURCES)
#error "Define `TOML11_COMPILE_SOURCES` before compiling source code!"
#endif

namespace toml
{
template class basic_value<type_config>;
template class basic_value<ordered_type_config>;
} // toml

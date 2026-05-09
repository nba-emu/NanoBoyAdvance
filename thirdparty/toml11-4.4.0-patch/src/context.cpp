#include <toml11/context.hpp>
#include <toml11/types.hpp>

#if ! defined(TOML11_COMPILE_SOURCES)
#error "Define `TOML11_COMPILE_SOURCES` before compiling source code!"
#endif

namespace toml
{
namespace detail
{
template class context<::toml::type_config>;
template class context<::toml::ordered_type_config>;
} // detail
} // toml

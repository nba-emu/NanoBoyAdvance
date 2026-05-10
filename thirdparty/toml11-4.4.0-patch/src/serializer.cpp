#include <toml11/serializer.hpp>
#include <toml11/types.hpp>

#if ! defined(TOML11_COMPILE_SOURCES)
#error "Define `TOML11_COMPILE_SOURCES` before compiling source code!"
#endif

namespace toml
{
struct type_config;
struct ordered_type_config;

template typename basic_value<type_config>::string_type
format<type_config>(const basic_value<type_config>&, const spec);

template typename basic_value<type_config>::string_type
format<type_config>(const typename basic_value<type_config>::key_type& k,
                    const basic_value<type_config>& v, const spec);

template typename basic_value<type_config>::string_type
format<type_config>(const std::vector<typename basic_value<type_config>::key_type>& ks,
                    const basic_value<type_config>& v, const spec s);

template typename basic_value<type_config>::string_type
format<ordered_type_config>(const basic_value<ordered_type_config>&, const spec);

template typename basic_value<type_config>::string_type
format<ordered_type_config>(const typename basic_value<ordered_type_config>::key_type& k,
                            const basic_value<ordered_type_config>& v, const spec);

template typename basic_value<type_config>::string_type
format<ordered_type_config>(const std::vector<typename basic_value<ordered_type_config>::key_type>& ks,
                            const basic_value<ordered_type_config>& v, const spec s);

namespace detail
{
template class serializer<::toml::type_config>;
template class serializer<::toml::ordered_type_config>;
} // detail
} // toml

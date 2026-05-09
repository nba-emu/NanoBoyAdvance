#include <toml11/parser.hpp>
#include <toml11/skip.hpp>
#include <toml11/types.hpp>

#if ! defined(TOML11_COMPILE_SOURCES)
#error "Define `TOML11_COMPILE_SOURCES` before compiling source code!"
#endif

namespace toml
{

namespace detail
{
template bool skip_whitespace            <type_config>(location& loc, const context<type_config>&);
template bool skip_empty_lines           <type_config>(location& loc, const context<type_config>&);
template void skip_comment_block         <type_config>(location& loc, const context<type_config>&);
template void skip_empty_or_comment_lines<type_config>(location& loc, const context<type_config>&);
template void skip_string_like           <type_config>(location& loc, const context<type_config>&);
template void skip_array_like            <type_config>(location& loc, const context<type_config>&);
template void skip_inline_table_like     <type_config>(location& loc, const context<type_config>&);
template void skip_value                 <type_config>(location& loc, const context<type_config>&);
template void skip_key_value_pair        <type_config>(location& loc, const context<type_config>&);
template void skip_until_next_table      <type_config>(location& loc, const context<type_config>&);

template bool skip_whitespace            <ordered_type_config>(location& loc, const context<ordered_type_config>&);
template bool skip_empty_lines           <ordered_type_config>(location& loc, const context<ordered_type_config>&);
template void skip_comment_block         <ordered_type_config>(location& loc, const context<ordered_type_config>&);
template void skip_empty_or_comment_lines<ordered_type_config>(location& loc, const context<ordered_type_config>&);
template void skip_string_like           <ordered_type_config>(location& loc, const context<ordered_type_config>&);
template void skip_array_like            <ordered_type_config>(location& loc, const context<ordered_type_config>&);
template void skip_inline_table_like     <ordered_type_config>(location& loc, const context<ordered_type_config>&);
template void skip_value                 <ordered_type_config>(location& loc, const context<ordered_type_config>&);
template void skip_key_value_pair        <ordered_type_config>(location& loc, const context<ordered_type_config>&);
template void skip_until_next_table      <ordered_type_config>(location& loc, const context<ordered_type_config>&);

} // detail
} // toml

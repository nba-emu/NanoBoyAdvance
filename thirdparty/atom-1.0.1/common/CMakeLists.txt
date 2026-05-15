# SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
# SPDX-License-Identifier: 0BSD

set(SOURCES
  src/panic.cc
)

set(HEADERS
)

set(HEADERS_PUBLIC
  include/atom/detail/parse_utils.hh
  include/atom/arena.hh
  include/atom/arguments.hh
  include/atom/bit.hh
  include/atom/bit_flags.hh
  include/atom/const_char_array.hh
  include/atom/float.hh
  include/atom/hash.hh
  include/atom/integer.hh
  include/atom/literal.hh
  include/atom/meta.hh
  include/atom/non_copyable.hh
  include/atom/panic.hh
  include/atom/punning.hh
  include/atom/result.hh
  include/atom/vector_n.hh
)

add_library(atom-common ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})
target_include_directories(atom-common PUBLIC include)
target_link_libraries(atom-common PUBLIC fmt::fmt)

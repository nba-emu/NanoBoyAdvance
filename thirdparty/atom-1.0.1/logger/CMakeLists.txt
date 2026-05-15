# SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
# SPDX-License-Identifier: 0BSD

set(SOURCES
  src/sink/console.cc
  src/sink/file.cc
  src/logger.cc
)

set(HEADERS
)

set(HEADERS_PUBLIC
  include/atom/logger/sink/console.hh
  include/atom/logger/sink/file.hh
  include/atom/logger/logger.hh
)

add_library(atom-logger ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})
target_link_libraries(atom-logger PUBLIC fmt::fmt atom-common)
target_include_directories(atom-logger PRIVATE src PUBLIC include)

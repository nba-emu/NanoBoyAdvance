cmake_minimum_required(VERSION 3.2)
project(platform-core CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  src/emulator.cpp
  src/frame_limiter.cpp
  src/game_db.cpp
)

set(HEADERS
  src/game_db.hpp
)

set(HEADERS_PUBLIC
  include/platform/emulator.hpp
  include/platform/frame_limiter.hpp
)

add_library(platform-core STATIC ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})
target_include_directories(platform-core PRIVATE src)
target_include_directories(platform-core PUBLIC include)

target_link_libraries(platform-core PUBLIC nba)
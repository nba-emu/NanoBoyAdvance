cmake_minimum_required(VERSION 3.2)
project(platform-core CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeModules)

include(FindSDL2)
find_package(SDL2 REQUIRED)

set(SOURCES
  src/device/sdl_audio_device.cpp
  src/bios_loader.cpp
  src/config.cpp
  src/emulator.cpp
  src/frame_limiter.cpp
  src/game_db.cpp
  src/rom_loader.cpp
)

set(HEADERS
)

set(HEADERS_PUBLIC
  include/platform/device/sdl_audio_device.hpp
  include/platform/bios_loader.hpp
  include/platform/config.hpp
  include/platform/emulator.hpp
  include/platform/frame_limiter.hpp
  include/platform/game_db.hpp
  include/platform/rom_loader.hpp
)

add_library(platform-core STATIC ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})
target_include_directories(platform-core PRIVATE src)
target_include_directories(platform-core PUBLIC include ${SDL2_INCLUDE_DIR})

target_link_libraries(platform-core PUBLIC nba toml11::toml11 ${SDL2_LIBRARY})
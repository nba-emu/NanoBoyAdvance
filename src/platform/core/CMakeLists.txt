option(USE_STATIC_SDL "Link against static version of SDL library." OFF)
option(USE_SYSTEM_TOML11 "Use system-provided toml11 library." OFF)

if(USE_STATIC_SDL)
  find_package(SDL2 2.0.10
    REQUIRED COMPONENTS SDL2-static
    OPTIONAL_COMPONENTS SDL2main
    CONFIG)
else()
  find_package(SDL2 2.0.10
    REQUIRED COMPONENTS SDL2
    OPTIONAL_COMPONENTS SDL2main
    CONFIG)
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

if(USE_SYSTEM_TOML11)
  find_package(toml11 3.7 REQUIRED)
else()
  include(FetchContent)
  FetchContent_Declare(toml11
    GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
    GIT_TAG        dcfe39a783a94e8d52c885e5883a6fbb21529019 # v3.7.1
  )
  set(toml11_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(toml11)
endif()

set(SOURCES
  src/device/ogl_video_device.cpp
  src/device/sdl_audio_device.cpp
  src/loader/bios.cpp
  src/loader/rom.cpp
  src/loader/save_state.cpp
  src/writer/save_state.cpp
  src/config.cpp
  src/emulator_thread.cpp
  src/frame_limiter.cpp
  src/game_db.cpp
)

set(HEADERS
  src/device/shader/color_higan.glsl.hpp
  src/device/shader/color_agb.glsl.hpp
  src/device/shader/common.glsl.hpp
  src/device/shader/lcd_ghosting.glsl.hpp
  src/device/shader/output.glsl.hpp
)

set(HEADERS_PUBLIC
  include/platform/device/ogl_video_device.hpp
  include/platform/device/sdl_audio_device.hpp
  include/platform/loader/bios.hpp
  include/platform/loader/rom.hpp
  include/platform/loader/save_state.hpp
  include/platform/writer/save_state.hpp
  include/platform/config.hpp
  include/platform/emulator_thread.hpp
  include/platform/frame_limiter.hpp
  include/platform/game_db.hpp
)

add_library(platform-core STATIC)
target_sources(platform-core PRIVATE ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})
target_include_directories(platform-core PRIVATE src PUBLIC include)

if(TARGET SDL2::SDL2main)
  target_link_libraries(platform-core PUBLIC SDL2::SDL2main)
endif()

if(USE_STATIC_SDL)
  target_link_libraries(platform-core PUBLIC SDL2::SDL2-static)
else()
  target_link_libraries(platform-core PUBLIC SDL2::SDL2)
endif()

target_link_libraries(platform-core
  PRIVATE unarr
  PUBLIC nba toml11::toml11 OpenGL::GL GLEW::GLEW
)

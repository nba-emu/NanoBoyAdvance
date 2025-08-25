option(USE_STATIC_SDL "Link against static version of SDL library." OFF)
option(USE_SYSTEM_GLAD "Use system-provided glad library." OFF)
option(USE_SYSTEM_TOML11 "Use system-provided toml11 library." OFF)
option(USE_SYSTEM_UNARR "Use system-provided unarr library." OFF)

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

include(FetchContent)

if(USE_SYSTEM_GLAD)
  find_package(Glad CONFIG REQUIRED)
else()
  FetchContent_Declare(glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        658f48e72aee3c6582e80b05ac0f8787a64fe6bb # v2.0.6
    SOURCE_SUBDIR  cmake
  )
  FetchContent_MakeAvailable(glad)
endif()

glad_add_library(glad_gl_core_33 STATIC
  LANGUAGE c REPRODUCIBLE API gl:core=3.3 EXTENSIONS NONE
)

if(USE_SYSTEM_TOML11)
  find_package(toml11 3.7 REQUIRED)
else()
  FetchContent_Declare(toml11
    GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
    GIT_TAG        d4eb5f3c9d8557b3820c80d55c41068839341b27 # v3.8.1
  )
  set(toml11_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(toml11)
endif()

if(USE_SYSTEM_UNARR)
  find_package(unarr REQUIRED)
else()
  set(USE_SYSTEM_BZ2 OFF CACHE BOOL "" FORCE)
  set(USE_SYSTEM_LZMA OFF CACHE BOOL "" FORCE)
  set(USE_SYSTEM_ZLIB OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(unarr
    GIT_REPOSITORY https://github.com/nba-emu/unarr.git
    GIT_TAG        839066857b68a78213510f47ccbe72e2945b1b1d # v1.1.1
  )
  FetchContent_GetProperties(unarr)
  if (NOT unarr_POPULATED)
    FetchContent_Populate(unarr)
    if (EXISTS ${unarr_SOURCE_DIR}/CMakeLists.txt)
      add_subdirectory(${unarr_SOURCE_DIR} ${unarr_BINARY_DIR} EXCLUDE_FROM_ALL)
      add_library(unarr::unarr ALIAS unarr)
    endif()
  endif()
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
  src/device/shader/lcd1x.glsl.hpp
  src/device/shader/output.glsl.hpp
  src/device/shader/sharp_bilinear.glsl.hpp
  src/device/shader/xbrz.glsl.hpp
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
  PRIVATE unarr::unarr
  PUBLIC nba toml11::toml11 OpenGL::GL glad_gl_core_33
)

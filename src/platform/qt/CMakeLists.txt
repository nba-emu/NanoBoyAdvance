
include(version.cmake)

set(SOURCES
  src/widget/debugger/ppu/background_viewer_window.cpp
  src/widget/debugger/ppu/background_viewer.cpp
  src/widget/debugger/ppu/sprite_viewer.cpp
  src/widget/debugger/ppu/sprite_viewer_window.cpp
  src/widget/debugger/ppu/color_grid.cpp
  src/widget/debugger/ppu/palette_viewer.cpp
  src/widget/debugger/ppu/palette_viewer_window.cpp
  src/widget/debugger/ppu/tile_viewer.cpp
  src/widget/debugger/ppu/tile_viewer_window.cpp
  src/widget/controller_manager.cpp
  src/widget/input_window.cpp
  src/widget/main_window.cpp
  src/widget/screen.cpp
  src/config.cpp
  src/main.cpp
)

set(HEADERS
  src/widget/debugger/ppu/background_viewer_window.hpp
  src/widget/debugger/ppu/background_viewer.hpp
  src/widget/debugger/ppu/color_grid.hpp
  src/widget/debugger/ppu/palette_viewer.hpp
  src/widget/debugger/ppu/palette_viewer_window.hpp
  src/widget/debugger/ppu/sprite_viewer.hpp
  src/widget/debugger/ppu/sprite_viewer_window.hpp
  src/widget/debugger/ppu/tile_viewer.hpp
  src/widget/debugger/ppu/tile_viewer_window.hpp
  src/widget/debugger/utility.hpp
  src/widget/controller_manager.hpp
  src/widget/input_window.hpp
  src/widget/main_window.hpp
  src/widget/screen.hpp
  src/config.hpp
  version.in.hpp
)

set(RESOURCES
)

set(CMAKE_AUTOMOC ON)

option(USE_QT6 "Use Qt 6" OFF)
option(PORTABLE_MODE "Portable Mode" ON)

if(USE_QT6)
  find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL REQUIRED)
  set(QT_DEPS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL)
else()
  find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)
  set(QT_DEPS Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
endif()

if(PORTABLE_MODE)
  add_compile_definitions(PORTABLE_MODE)
endif()

add_executable(NanoBoyAdvance-Qt)

# Append application resources (icon and manifest) on Win32.
if(WIN32)
  list(APPEND RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/app.rc)

  set_target_properties(NanoBoyAdvance-Qt PROPERTIES WIN32_EXECUTABLE ON)

  if(MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -i <SOURCE> -o <OBJECT>")
  endif()
endif()

if(APPLE)
  option(MACOS_BUILD_APP_BUNDLE "Build an app bundle" OFF)
endif()

if(APPLE AND MACOS_BUILD_APP_BUNDLE)
  option(MACOS_BUNDLE_QT "Bundle Qt with the app bundle" ON)

  set(MACOSX_BUNDLE_BUNDLE_NAME "NanoBoyAdvance")
  set(MACOSX_BUNDLE_ICON_FILE "NanoBoyAdvance.icns")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.nba-emu.NanoBoyAdvance")
  set(MACOSX_BUNDLE_INFO_STRING "Game Boy Advance emulator")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

  list(APPEND RESOURCES rc/NanoBoyAdvance.icns)
  set_target_properties(NanoBoyAdvance-Qt PROPERTIES
    RESOURCE rc/NanoBoyAdvance.icns
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/rc/Info.plist.in"
  )
  target_compile_definitions(NanoBoyAdvance-Qt PRIVATE MACOS_BUILD_APP_BUNDLE)

  if(MACOS_BUNDLE_QT)
    if(USE_QT6)
      get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    else()
      get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
    endif()
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

    add_custom_command(TARGET NanoBoyAdvance-Qt POST_BUILD
      COMMAND "${MACDEPLOYQT_EXECUTABLE}" NanoBoyAdvance.app -always-overwrite
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/qt
      COMMENT "Deploying Qt..."
    )
  endif()
endif()

target_sources(NanoBoyAdvance-Qt PRIVATE ${SOURCES} ${HEADERS} ${RESOURCES})

if(WIN32)
  option(PLATFORM_QT_STATIC "Build statically linked exectuable (Qt)" OFF)

  if(PLATFORM_QT_STATIC)
    target_link_options(NanoBoyAdvance-Qt PRIVATE -static)
  endif()
endif()

set_target_properties(NanoBoyAdvance-Qt PROPERTIES OUTPUT_NAME "NanoBoyAdvance")

target_link_libraries(NanoBoyAdvance-Qt PRIVATE platform-core ${QT_DEPS})
target_include_directories(NanoBoyAdvance-Qt PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "rc/config.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

if(WIN32 AND PLATFORM_QT_STATIC)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  target_link_libraries(NanoBoyAdvance-Qt PRIVATE z zstd $<$<CXX_COMPILER_ID:GNU>:gcc stdc++ winpthread>)
endif()

install(TARGETS NanoBoyAdvance-Qt DESTINATION bin)
# Variables LINUX and BSD were introduced only with CMake 3.25.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  install(FILES rc/io.nanoboyadvance.NanoBoyAdvance.png DESTINATION share/icons/hicolor/128x128/apps)
  install(FILES rc/io.nanoboyadvance.NanoBoyAdvance.desktop DESTINATION share/applications)
  install(FILES rc/io.nanoboyadvance.NanoBoyAdvance.metainfo.xml DESTINATION share/metainfo)
endif()

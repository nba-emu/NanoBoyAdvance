# SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
# SPDX-License-Identifier: GPL-3.0-or-later

include(version.cmake)

cmake_dependent_option(MACOS_BUILD_APP_BUNDLE "Package as an app bundle" ON APPLE OFF)
cmake_dependent_option(WINDOWS_DEPLOY_QT "Deploys Qt and other libraries on Windows" ON WIN32 OFF)

cmake_dependent_option(PORTABLE_MODE "Enable portable mode" OFF MACOS_BUILD_APP_BUNDLE ON)

set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL REQUIRED)

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
  ${CMAKE_CURRENT_BINARY_DIR}/version.hpp
)

set(RESOURCES
  # ...
)

if(PORTABLE_MODE)
  add_compile_definitions(PORTABLE_MODE)
endif()

add_executable(NanoBoyAdvance-Qt)

target_compile_definitions(NanoBoyAdvance-Qt PRIVATE $<$<NOT:$<CONFIG:Debug>>:IS_RELEASE>)

# Append application resources (icon and manifest) on Win32.
if(WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rc/app.rc.in ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
  list(APPEND RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/app.rc ${CMAKE_CURRENT_LIST_DIR}/rc/app.manifest)
  target_compile_definitions(NanoBoyAdvance-Qt PRIVATE SDL_MAIN_HANDLED=1)
  set_target_properties(NanoBoyAdvance-Qt PROPERTIES WIN32_EXECUTABLE ON)

  if(WINDOWS_DEPLOY_QT)
    get_target_property(WINDEPLOYQT_EXECUTABLE Qt6::windeployqt IMPORTED_LOCATION)
    add_custom_command(TARGET NanoBoyAdvance-Qt POST_BUILD COMMAND "${WINDEPLOYQT_EXECUTABLE}" NanoBoyAdvance.exe WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> COMMENT "Deploying Qt...")
  endif()
endif()

if(APPLE AND MACOS_BUILD_APP_BUNDLE)
  set(MACOSX_BUNDLE_BUNDLE_NAME "NanoBoyAdvance")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

  list(APPEND RESOURCES rc/NanoBoyAdvance.icns)
  set_target_properties(NanoBoyAdvance-Qt PROPERTIES RESOURCE rc/NanoBoyAdvance.icns MACOSX_BUNDLE ON MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/rc/Info.plist.in")
  target_compile_definitions(NanoBoyAdvance-Qt PRIVATE MACOS_BUILD_APP_BUNDLE)

  get_target_property(MACDEPLOYQT_EXECUTABLE Qt6::macdeployqt IMPORTED_LOCATION)
  add_custom_command(TARGET NanoBoyAdvance-Qt POST_BUILD COMMAND "${MACDEPLOYQT_EXECUTABLE}" NanoBoyAdvance.app -always-overwrite WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> COMMENT "Deploying Qt...")
endif()

target_sources(NanoBoyAdvance-Qt PRIVATE ${SOURCES} ${HEADERS} ${RESOURCES})

set_target_properties(NanoBoyAdvance-Qt PROPERTIES OUTPUT_NAME "NanoBoyAdvance")
target_link_libraries(NanoBoyAdvance-Qt PRIVATE platform-core Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL)
target_include_directories(NanoBoyAdvance-Qt PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})

if(APPLE AND MACOS_BUILD_APP_BUNDLE)
  install(TARGETS NanoBoyAdvance-Qt BUNDLE DESTINATION Applications)
else()
  install(TARGETS NanoBoyAdvance-Qt DESTINATION bin)
endif()

# Variables LINUX and BSD were introduced only with CMake 3.25.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  install(FILES rc/io.nanoboyadvance.NanoBoyAdvance.png DESTINATION share/icons/hicolor/128x128/apps)
  install(FILES rc/io.nanoboyadvance.NanoBoyAdvance.desktop DESTINATION share/applications)
endif()

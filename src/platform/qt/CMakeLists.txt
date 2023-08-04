project(NanoBoyAdvance-Qt CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  src/widget/background_viewer_window.cpp
  src/widget/background_viewer.cpp
  src/widget/controller_manager.cpp
  src/widget/input_window.cpp
  src/widget/main_window.cpp
  src/widget/screen.cpp
  src/widget/palette_box.cpp
  src/widget/palette_viewer_window.cpp
  src/config.cpp
  src/main.cpp
)

set(HEADERS
  src/widget/background_viewer_window.hpp
  src/widget/background_viewer.hpp
  src/widget/controller_manager.hpp
  src/widget/input_window.hpp
  src/widget/main_window.hpp
  src/widget/palette_box.hpp
  src/widget/palette_viewer_window.hpp
  src/widget/screen.hpp
  src/config.hpp
  version.in.hpp
)

set(RESOURCES
)

set(CMAKE_AUTOMOC ON)

option(USE_QT6 "Use Qt 6" OFF)

if (USE_QT6)
  find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets REQUIRED)
  set(QT_DEPS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
else()
  find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)
  set(QT_DEPS Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
endif()

# Append application resources (icon and manifest) on Win32.
if(WIN32)
  list(APPEND RESOURCES rc/app.rc)

  if(MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -i <SOURCE> -o <OBJECT>")
  endif()  
endif()

if (APPLE)
  option(MACOS_BUILD_APP_BUNDLE "Build an app bundle" OFF)
endif()

if (APPLE AND MACOS_BUILD_APP_BUNDLE)
  option(MACOS_BUNDLE_QT "Bundle Qt with the app bundle" ON)

  set(MACOSX_BUNDLE_BUNDLE_NAME "NanoBoyAdvance")
  set(MACOSX_BUNDLE_ICON_FILE "NanoBoyAdvance.icns")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.nba-emu.NanoBoyAdvance")
  set(MACOSX_BUNDLE_INFO_STRING "Game Boy Advance emulator")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.7.1")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.7.1")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "1.7.1")

  list(APPEND RESOURCES rc/NanoBoyAdvance.icns)
  add_executable(NanoBoyAdvance-Qt MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES})
  set_target_properties(NanoBoyAdvance-Qt PROPERTIES
	  RESOURCE rc/NanoBoyAdvance.icns
	  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/rc/Info.plist.in"
  )
  target_compile_definitions(NanoBoyAdvance-Qt PRIVATE MACOS_BUILD_APP_BUNDLE)

  if (MACOS_BUNDLE_QT)
    if (USE_QT6)
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
else()
  add_executable(NanoBoyAdvance-Qt ${SOURCES} ${HEADERS} ${RESOURCES})
endif()

if (WIN32)
  option(PLATFORM_QT_STATIC "Build statically linked exectuable (Qt)" OFF)

  if(PLATFORM_QT_STATIC)
    target_link_options(NanoBoyAdvance-Qt PRIVATE -static)
  endif()
endif()

set_target_properties(NanoBoyAdvance-Qt PROPERTIES OUTPUT_NAME "NanoBoyAdvance")

target_link_libraries(NanoBoyAdvance-Qt platform-core ${QT_DEPS})
target_include_directories(NanoBoyAdvance-Qt PRIVATE src)

file(COPY "rc/config.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

if(WIN32 AND PLATFORM_QT_STATIC)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  target_link_libraries(NanoBoyAdvance-Qt -static z zstd gcc stdc++ winpthread winmm version Imm32 Setupapi)
endif()

include(version.cmake)

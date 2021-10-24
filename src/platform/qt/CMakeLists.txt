project(NanoBoyAdvance-Qt CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  src/widget/input_window.cpp
  src/widget/main_window.cpp
  src/widget/screen.cpp
  src/config.cpp
  src/main.cpp
)

set(HEADERS
  src/widget/input_window.hpp
  src/widget/main_window.hpp
  src/widget/screen.hpp
  src/config.hpp
)

set(RESOURCES
)

set(CMAKE_AUTOMOC ON)

if(WIN32)
  option(PLATFORM_QT_STATIC "Build statically linked exectuable (Qt)" OFF)
  
  # Shamelessly stolen from melonDS:
  if(PLATFORM_QT_STATIC AND QT5_STATIC_DIR)
    set(QT5_STATIC_BASE ${QT5_STATIC_DIR}/lib/cmake/Qt5)
    set(Qt5_DIR ${QT5_STATIC_BASE})
    set(Qt5Core_DIR ${QT5_STATIC_BASE}Core)
    set(Qt5Gui_DIR ${QT5_STATIC_BASE}Gui)
    set(Qt5Widgets_DIR ${QT5_STATIC_BASE}Widgets)
    set(Qt5OpenGL_DIR ${QT5_STATIC_BASE}OpenGL)
  endif()
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)
set(QT_DEPS Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)

# Append application resources (icon and manifest) on Win32.
if(WIN32)
  list(APPEND RESOURCES rc/app.rc)

  if(MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -i <SOURCE> -o <OBJECT>")
  endif()  
endif()

add_executable(NanoBoyAdvance-Qt ${SOURCES} ${HEADERS} ${RESOURCES})
target_link_libraries(NanoBoyAdvance-Qt platform-core ${QT_DEPS})
target_include_directories(NanoBoyAdvance-Qt PRIVATE src)
set_target_properties(NanoBoyAdvance-Qt PROPERTIES OUTPUT_NAME "NanoBoyAdvance")

file(COPY "rc/config.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

if(WIN32 AND PLATFORM_QT_STATIC)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  target_link_libraries(NanoBoyAdvance-Qt -static z zstd gcc stdc++ winpthread winmm version Imm32 Setupapi)
endif()
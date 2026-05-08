cmake_minimum_required(VERSION 3.20)
project(NanoBoyAdvance LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED Python_ROOT_DIR AND APPLE)
  set(Python_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/.venv)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND APPLE)
  # Removes annoying warnings about deprecations for fmt on macOS
  add_compile_options(-Wno-deprecated-declarations -Wno-deprecated-literal-operator)
endif()

option(PLATFORM_QT "Build Qt frontend." ON)

add_subdirectory(src/nba)
add_subdirectory(src/platform/core)

if (PLATFORM_QT)
  add_subdirectory(src/platform/qt ${CMAKE_CURRENT_BINARY_DIR}/bin/qt/)
endif()

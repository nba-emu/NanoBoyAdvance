
find_package(Git)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 8)
set(VERSION_PATCH 2)

option(RELEASE_BUILD "Build a release version" OFF)

if(GIT_FOUND)
  # https://stackoverflow.com/questions/1435953/how-can-i-pass-git-sha1-to-compiler-as-definition-using-cmake
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE VERSION_GIT_BRANCH)

  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --match=NeVeRmAtCh --always --dirty
    OUTPUT_VARIABLE VERSION_GIT_HASH)

  string(STRIP "${VERSION_GIT_BRANCH}" VERSION_GIT_BRANCH)
  string(STRIP "${VERSION_GIT_HASH}" VERSION_GIT_HASH)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.in.hpp" "${CMAKE_CURRENT_BINARY_DIR}/version.hpp")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rc/app.rc ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
cmake_minimum_required(VERSION 3.16)

# project_source_dir has not been set yet
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/toml11/version.hpp" TOML11_MAJOR_VERSION_STRING
    REGEX "#define TOML11_VERSION_MAJOR ([0-9]+)")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/toml11/version.hpp" TOML11_MINOR_VERSION_STRING
    REGEX "#define TOML11_VERSION_MINOR ([0-9]+)")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/toml11/version.hpp" TOML11_PATCH_VERSION_STRING
    REGEX "#define TOML11_VERSION_PATCH ([0-9]+)")

string(REGEX REPLACE "#define TOML11_VERSION_MAJOR ([0-9]+)" "\\1" TOML11_VERSION_MAJOR "${TOML11_MAJOR_VERSION_STRING}")
string(REGEX REPLACE "#define TOML11_VERSION_MINOR ([0-9]+)" "\\1" TOML11_VERSION_MINOR "${TOML11_MINOR_VERSION_STRING}")
string(REGEX REPLACE "#define TOML11_VERSION_PATCH ([0-9]+)" "\\1" TOML11_VERSION_PATCH "${TOML11_PATCH_VERSION_STRING}")

project(toml11 LANGUAGES CXX VERSION "${TOML11_VERSION_MAJOR}.${TOML11_VERSION_MINOR}.${TOML11_VERSION_PATCH}")

include(CTest) # to use ${BUILD_TESTING}

option(TOML11_PRECOMPILE "precompile toml11 library" OFF)
option(TOML11_ENABLE_ACCESS_CHECK "enable access check feature (beta)" OFF)

include(CMakeDependentOption)
cmake_policy(PUSH)
#if(POLICY CMP0127)
#cmake_policy(SET CMP0127 OLD) # syntax of condition changed in 3.22
#endif()
cmake_dependent_option(TOML11_INSTALL "install toml11 library" ON
    "${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME}" OFF)
cmake_dependent_option(TOML11_BUILD_EXAMPLES "build toml11 examples" OFF
    "${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME}" OFF)
cmake_dependent_option(TOML11_BUILD_TESTS "build toml11 unit tests" OFF
    "${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME}; ${BUILD_TESTING}" OFF)
cmake_dependent_option(TOML11_BUILD_TOML_TESTS "build toml11 toml-test encoder & decoder" OFF
    "${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME}" OFF)
cmake_policy(POP)

cmake_dependent_option(TOML11_TEST_WITH_ASAN  "build toml11 unit tests with asan" OFF
    "${TOML11_BUILD_TESTS}" OFF)
cmake_dependent_option(TOML11_TEST_WITH_UBSAN "build toml11 unit tests with ubsan" OFF
    "${TOML11_BUILD_TESTS}" OFF)

if(${TOML11_TEST_WITH_ASAN} AND ${TOML11_TEST_WITH_UBSAN})
    message(FATAL_ERROR "trying to build tests with BOTH asan and ubsan")
endif()

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-Wall"                 TOML11_COMPILER_SUPPORTS_WALL)
check_cxx_compiler_flag("-Wextra"               TOML11_COMPILER_SUPPORTS_WEXTRA)
check_cxx_compiler_flag("-Wpedantic"            TOML11_COMPILER_SUPPORTS_WPEDANTIC)
check_cxx_compiler_flag("-Werror"               TOML11_COMPILER_SUPPORTS_WERROR)
check_cxx_compiler_flag("-Wsign-conversion"     TOML11_COMPILER_SUPPORTS_WSIGN_CONVERSION)
check_cxx_compiler_flag("-Wconversion"          TOML11_COMPILER_SUPPORTS_WCONVERSION)
check_cxx_compiler_flag("-Wduplicated-cond"     TOML11_COMPILER_SUPPORTS_WDUPLICATED_COND)
check_cxx_compiler_flag("-Wduplicated-branches" TOML11_COMPILER_SUPPORTS_WDUPLICATED_BRANCHES)
check_cxx_compiler_flag("-Wlogical-op"          TOML11_COMPILER_SUPPORTS_WLOGICAL_OP)
check_cxx_compiler_flag("-Wdouble-promotion"    TOML11_COMPILER_SUPPORTS_WDOUBLE_PROMOTION)
check_cxx_compiler_flag("-Wrange-loop-analysis" TOML11_COMPILER_SUPPORTS_WRANGE_LOOP_ANALYSIS)
check_cxx_compiler_flag("-Wundef"               TOML11_COMPILER_SUPPORTS_WUNDEF)
check_cxx_compiler_flag("-Wshadow"              TOML11_COMPILER_SUPPORTS_WSHADOW)

include(GNUInstallDirs)
set(TOML11_INSTALL_CMAKE_DIR   ${CMAKE_INSTALL_LIBDIR}/cmake/toml11)
set(TOML11_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(TOML11_CONFIG_DIR          ${CMAKE_CURRENT_BINARY_DIR}/cmake)
set(TOML11_CONFIG              ${TOML11_CONFIG_DIR}/toml11Config.cmake)
set(TOML11_CONFIG_VERSION      ${TOML11_CONFIG_DIR}/toml11ConfigVersion.cmake)

# root project?
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 11)
    endif()
    if(NOT DEFINED CMAKE_CXX_EXTENSIONS)
        set(CMAKE_CXX_EXTENSIONS OFF)
    endif()
    if(NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
    endif()

    if(${TOML11_BUILD_TESTS} OR ${TOML11_BUILD_TOML_TESTS})
        add_subdirectory(tests)
    endif()

    if(${TOML11_BUILD_EXAMPLES})
        add_subdirectory(examples)
    endif()
endif()

add_subdirectory(src)

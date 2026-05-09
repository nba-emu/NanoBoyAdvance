+++
title = "installation"
type  = "docs"
weight = 1
+++

# Installation

## Using `single_include`

The `ingle_include/toml.hpp`s is a single-file, header-only library that consolidates all the functionalities of toml11 into one file.

The simplest way to use it is to copy this file to a location included in your `INCLUDE_PATH` and then add `#include <toml.hpp>` to your source code.

The MIT license notice is included in both the comments and the `toml::license_notice()` function.
If you are redistributing the software without releasing the source code, you must either copy the toml11 license file and include it with your distribution, or ensure that the `toml::license_notice()` function can be called.

## Cloning toml11 and using cmake

If you place toml11 within your repository, for example by using git submodule, and you are using cmake, you can make use of it by adding `add_subdirectory(toml11)` to your `CMakeLists.txt`.

```cmake
add_subdirectory(toml11)
add_executable(main main.cpp)
target_link_libraries(main PUBLIC toml11::toml11)
```

toml11 will only run tests and install when it is the root project.

### CMake `FetchContent`

Using `FetchContent`, you can automatically download toml11 to your `build` directory.

```cmake
include(FetchContent)
FetchContent_Declare(
  toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
  GIT_TAG        v4.4.0
)
FetchContent_MakeAvailable(toml11)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE toml11::toml11)
```

### CMake Package Manager (CPM)

After [adding cpm to your project](https://github.com/cpm-cmake/CPM.cmake?tab=readme-ov-file#adding-cpm), you can use toml11 by doing:

```cmake
include(cmake/CPM.cmake)

CPMAddPackage("gh:ToruNiina/toml11@4.4.0")

# OR

CPMAddPackage(
    NAME toml11
    GITHUB_REPOSITORY "ToruNiina/toml11"
    VERSION 4.4.0
    OPTIONS
    "TOML11_PRECOMPILE ON" # to pre-compile
    "TOML11_ENABLE_ACCESS_CHECK ON" # to use value.accessed()
    )

add_executable(main main.cpp)
target_link_libraries(main PUBLIC toml11::toml11)
```

## Installing using cmake

After cloning toml11, you can install it using cmake.

```console
$ cmake -B ./build/ -DTOML11_BUILD_TESTS=ON
$ cmake --install ./build/ --prefix=/opt/toml11
```

If you want to run the test programs before installation, make sure to set `-DTOML11_BUILD_TESTS=ON` first.

Once the installation is complete, you can use it as follows:

```cmake
find_package(toml11)
add_executable(main main.cpp)
target_link_libraries(main PRIVATE toml11::toml11)
```

## Compiling with cmake to Create a Static Library

By defining `-DTOML11_PRECOMPILE=ON` when running cmake, you can precompile some of the functions in toml11, thereby reducing the overall compile time.

```console
$ cmake -B ./build/ -DTOML11_PRECOMPILE=ON
```

When linking the library, use `target_link_libraries` in CMake

```cmake
target_link_libraries(your_target PUBLIC toml11::toml11)
```

or pass `-DTOML11_COMPILE_SOURCES` to the compiler to suppress header-only features.

However, since toml11 supports multiple C++ versions and may switch types based on the value of `__cplusplus`,
there is a possibility of link failures if the version used during build differs from the version used during usage.
If you encounter issues, set the required version using `CMAKE_CXX_STANDARD` during compilation.
If this is difficult, use it as a header-only library as usual.

The `find_package(toml11)` command defines `TOML11_INCLUDE_DIR`.
Even if you install it as a precompiled library, you can still use it as a header-only library by adding `TOML11_INCLUDE_DIR` to `target_include_directories` and avoiding the use of `target_link_libraries`.

```cmake
find_package(toml11)
add_executable(main main.cpp)
# Include only, do not link
target_include_directories(main PRIVATE ${TOML11_INCLUDE_DIR})
```

## Compiling Examples

You can compile the `examples/` directory by setting `-DTOML11_BUILD_EXAMPLES=ON`.

```console
$ cmake -B ./build/ -DTOML11_BUILD_EXAMPLES=ON
$ cmake --build ./build/
```

The executable binaries for the examples will be generated in the `examples/` directory.

## Running Tests

To build the tests, set `-DTOML11_BUILD_TESTS=ON`.

```console
$ git submodule update --init --recursive
$ cmake -B ./build/ -DTOML11_BUILD_TESTS=ON
$ cmake --build ./build/
$ ctest --test-dir ./build/
```

To run the `toml-lang/toml-tests`, set `-DTOML11_BUILD_TOML_TESTS=ON`. This will build `toml11_decoder` and `toml11_encoder` in the `tests/` directory.

```console
$ git submodule update --init --recursive
$ cmake -B ./build/ -DTOML11_BUILD_TOML_TESTS=ON
$ cmake --build ./build/
$ ctest --test-dir ./build/
```


include(FetchContent)

FetchContent_Declare(uni-algo
  GIT_REPOSITORY https://github.com/uni-algo/uni-algo.git
  GIT_SHALLOW ON  # Download the branch without its history
  GIT_TAG v1.0.0) # The version you want to download

# Be aware that FetchContent_MakeAvailable requires CMake 3.14 or higher
FetchContent_MakeAvailable(uni-algo)

add_executable(canonicalize canonicalize.cpp)
target_link_libraries(canonicalize PRIVATE toml11::toml11 uni-algo::uni-algo)
set_target_properties(canonicalize PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")


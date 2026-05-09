include(FetchContent)

FetchContent_Declare(
  boost_ext_reflect
  GIT_REPOSITORY https://github.com/boost-ext/reflect
  GIT_SHALLOW ON  # Download the branch without its history
  GIT_TAG        v1.1.1
)
FetchContent_MakeAvailable(boost_ext_reflect)

add_executable(reflect reflect.cpp)
target_link_libraries(reflect PRIVATE toml11::toml11)
target_include_directories(reflect PRIVATE
    ${boost_ext_reflect_SOURCE_DIR}
    )
target_compile_features(reflect PRIVATE cxx_std_20)
set_target_properties(reflect PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

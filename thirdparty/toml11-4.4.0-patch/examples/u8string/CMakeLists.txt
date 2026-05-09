add_executable(u8string u8string.cpp)
target_link_libraries(u8string PRIVATE toml11::toml11)
target_compile_features(u8string PRIVATE cxx_std_20)
set_target_properties(u8string PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(spec_example spec_example.cpp)
target_link_libraries(spec_example PRIVATE toml11::toml11)
set_target_properties(spec_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(key_example key_example.cpp)
target_link_libraries(key_example PRIVATE toml11::toml11)
set_target_properties(key_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(array_example array_example.cpp)
target_link_libraries(array_example PRIVATE toml11::toml11)
set_target_properties(array_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(array_of_tables_example array_of_tables_example.cpp)
target_link_libraries(array_of_tables_example PRIVATE toml11::toml11)
set_target_properties(array_of_tables_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

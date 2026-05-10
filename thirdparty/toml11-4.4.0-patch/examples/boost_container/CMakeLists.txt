find_package(Boost 1.81.0)

if(Boost_FOUND)
    add_executable(container container.cpp)
    target_link_libraries(container PRIVATE toml11::toml11 Boost::boost)
    set_target_properties(container PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

find_package(Boost 1.67.0)

if(Boost_FOUND)
    add_executable(multiprecision multiprecision.cpp)
    target_link_libraries(multiprecision PRIVATE toml11::toml11 Boost::boost)
    set_target_properties(multiprecision PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

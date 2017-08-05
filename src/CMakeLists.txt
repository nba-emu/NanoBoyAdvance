include_directories(.)

# Add core libs
add_subdirectory(util)
add_subdirectory(core)
add_subdirectory(third_party)

if (SDL2_GUI)
    add_subdirectory(sdl)
endif()

include_directories(.)

# Add core libs
add_subdirectory(common)
add_subdirectory(gba)

if (SDL2)
    add_subdirectory(sdl)
endif()

if (QT_GUI)
    add_subdirectory(qt)
endif()
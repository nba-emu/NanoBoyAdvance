include_directories(.)

# Add core libs
add_subdirectory(util)
add_subdirectory(gba)
add_subdirectory(audio)
add_subdirectory(config)

if (SDL2_GUI)
    add_subdirectory(sdl)
endif()

if (QT_GUI)
    add_subdirectory(qt)
endif()

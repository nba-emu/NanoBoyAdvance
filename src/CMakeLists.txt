include_directories(.)

# Add core libs
add_subdirectory(util)
add_subdirectory(arm)
add_subdirectory(gba)
add_subdirectory(third_party)

if (GTK_GUI)
    add_subdirectory(gtk)
endif()

if (SDL2_GUI)
    add_subdirectory(sdl)
endif()

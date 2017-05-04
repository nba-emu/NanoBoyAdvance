# Search GTKMM 3.0 and GTKGLEXTMM-1.2
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM      gtkmm-2.4)
pkg_check_modules(GTKGLEXTMM gtkglextmm-1.2)

# Properly include/link GTKMM
link_directories(${GTKMM_LIBRARY_DIRS} ${GTKGLEXTMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS} ${GTKGLEXTMM_INCLUDE_DIRS})

# Include OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

set(SRCS
    main.cpp
    main_window.cpp
    controls/screen.cpp
)

set (HEADERS
    main_window.hpp
    controls/screen.hpp
)

# Create Executable
add_executable(NanoboyAdvance ${SRCS} ${HEADERS})

# Link it against required libraries
target_link_libraries(NanoboyAdvance core ${GTKMM_LIBRARIES} ${GTKGLEXTMM_LIBRARIES} ${PNG_LIBRARY})

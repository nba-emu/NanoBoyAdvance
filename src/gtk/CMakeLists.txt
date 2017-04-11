# Search GTKMM 3.0
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-3.0)

# Properly include/link GTKMM
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

set(SRCS
    main.cpp
)

set (HEADERS

)

# Create Executable
add_executable(NanoboyAdvance ${SRCS} ${HEADERS})

# Link it against required libraries
target_link_libraries(NanoboyAdvance gba ${GTKMM_LIBRARIES} ${PNG_LIBRARY})

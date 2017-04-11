set(SRCS
    main.cpp
)

set (HEADERS

)

# Properly include/link GTKMM
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

add_executable(NanoboyAdvance ${SRCS} ${HEADERS})

target_link_libraries(NanoboyAdvance gba ${GTKMM_LIBRARIES} ${PNG_LIBRARY})

set(SRCS
    arguments.cpp
    main.cpp
)

set(HEADERS
    arguments.h
)

add_executable(nanoboyadvance ${SRCS} ${HEADERS})
target_link_libraries(nanoboyadvance gba)
target_link_libraries(nanoboyadvance ${SDL2_LIBRARY} ${PNG_LIBRARY})
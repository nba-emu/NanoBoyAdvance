set(SRCS
    cmdline.cpp
    main.cpp
)

set(HEADERS
    cmdline.h
)

add_executable(nanoboyadvance ${SRCS} ${HEADERS})
target_link_libraries(nanoboyadvance gba)
target_link_libraries(nanoboyadvance ${SDL2_LIBRARY} ${PNG_LIBRARY})
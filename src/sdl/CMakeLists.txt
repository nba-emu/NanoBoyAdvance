set(SRCS
    arguments.cpp
    main.cpp
)

set(HEADERS
    arguments.h
)

add_executable(NanoboyAdvance-SDL ${SRCS} ${HEADERS})
target_link_libraries(NanoboyAdvance-SDL gba)
target_link_libraries(NanoboyAdvance-SDL ${SDL2_LIBRARY} ${PNG_LIBRARY})
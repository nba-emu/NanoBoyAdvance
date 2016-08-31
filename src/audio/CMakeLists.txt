set(SRCS
    sdl_adapter.cpp
)

set(HEADERS
    adapter.h
    sdl_adapter.h
)

add_library(audio STATIC ${SRCS} ${HEADERS})
target_link_libraries(audio ${SDL2_LIBRARY})

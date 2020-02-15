set(SOURCES
    main.cpp
)

set(HEADERS
)

include(FindSDL2 REQUIRED)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(NanoboyAdvance-SDL ${SOURCES} ${HEADERS})
target_link_libraries(NanoboyAdvance-SDL gba ${SDL2_LIBRARY})

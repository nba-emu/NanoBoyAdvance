set(SOURCES
    main.cpp
)

set(HEADERS
    device/audio_device.hpp
    device/input_device.hpp
    device/video_device.hpp
)

include(FindSDL2 REQUIRED)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(NanoboyAdvance-SDL ${SOURCES} ${HEADERS})
target_link_libraries(NanoboyAdvance-SDL nba ${SDL2_LIBRARY})

file(COPY "${PROJECT_SOURCE_DIR}/config.toml" DESTINATION "${PROJECT_BINARY_DIR}/source/platform/sdl/")
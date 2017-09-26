set(SRCS
    main.cpp
)

set(HEADERS
    argument_parser.hpp
)

add_executable(NanoboyAdvance-SDL ${SRCS} ${HEADERS})
target_link_libraries(NanoboyAdvance-SDL core)
target_link_libraries(NanoboyAdvance-SDL ${SDL2_LIBRARY})
# target_link_libraries(NanoboyAdvance-SDL ${SDL2_LIBRARY} ${PNG_LIBRARY})

file(COPY "${PROJECT_SOURCE_DIR}/config.ini" DESTINATION "${PROJECT_BINARY_DIR}/src/sdl/")

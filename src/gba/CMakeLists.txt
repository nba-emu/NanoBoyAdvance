set(SOURCES
    cpu.cpp
)

set(HEADERS
    cpu.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba armigo util)

set(SOURCES

)

set(HEADERS

)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba armigo util)

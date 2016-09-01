set(SRCS
    config.cpp
)

set(HEADERS
    config.h
)

add_library(config STATIC ${SRCS} ${HEADERS})

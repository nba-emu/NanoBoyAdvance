set(SRCS
    file.cpp
)

set(HEADERS
    file.h
    log.h
    types.h
)

add_library(util STATIC ${SRCS} ${HEADERS})
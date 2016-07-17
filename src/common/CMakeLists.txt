set(SRCS
    file.cpp
)

set(HEADERS
    file.h
    log.h
    types.h
)

add_library(common STATIC ${SRCS} ${HEADERS})
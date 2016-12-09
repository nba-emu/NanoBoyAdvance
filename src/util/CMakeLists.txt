set(SRCS
    file.cpp
)

set(HEADERS
    file.hpp
    log.h
    integer.hpp
)

add_library(util STATIC ${SRCS} ${HEADERS})

set(SRCS
    file.cpp
)

set(HEADERS
    file.hpp
    integer.hpp
    bit.hpp
    log.h
)

add_library(util STATIC ${SRCS} ${HEADERS})

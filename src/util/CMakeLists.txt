set(SRCS
    file.cpp
)

set(HEADERS
    file.hpp
    integer.hpp
    bit.hpp
)

add_library(util STATIC ${SRCS} ${HEADERS})

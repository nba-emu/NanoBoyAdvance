set(SRCS
    file.cpp
    logger.cpp
)

set(HEADERS
    file.hpp
    integer.hpp
    bit.hpp
    logger.hpp
)

add_library(util STATIC ${SRCS} ${HEADERS})
target_link_libraries(util fmt)

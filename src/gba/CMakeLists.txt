set(SRCS
    gba.cpp
    arm.cpp
    arm_instr.cpp
    thumb_instr.cpp
    memory.cpp
    dma.cpp
    timer.cpp
    video.cpp
    flash.cpp
    sram.cpp
)

set(HEADERS
    gba.h
    arm.h
    iodef.h
    interrupt.h
    memory.h
    video.h
    backup.h
    flash.h
    sram.h
    composer.h
)

add_library(gba STATIC ${SRCS} ${HEADERS})
target_link_libraries(gba util)

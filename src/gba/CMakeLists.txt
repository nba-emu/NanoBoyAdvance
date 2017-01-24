set(SRCS
    gba.cpp
    arm_gba.cpp
    io.cpp
    interrupt.cpp
    dma.cpp
    timer.cpp
    memory.cpp
    backup/flash.cpp
    backup/sram.cpp
    video/video.cpp
    video/composite.cpp
    video/renderers/text_mode.cpp
    video/renderers/rotate_scale.cpp
    video/renderers/sprite.cpp
    audio/audio.cpp
    audio/quadchannel.cpp
    audio/wavechannel.cpp
)

set(HEADERS
    gba.h
    arm_gba.hpp
    io.hpp
    interrupt.hpp
    dma.h
    timer.hpp
    memory.h
    backup/backup.h
    backup/flash.h
    backup/sram.h
    video/video.h
    video/video.inl
    audio/audio.h
    audio/fifo.h
    audio/quadchannel.h
    audio/wavechannel.h
)

add_library(gba STATIC ${SRCS} ${HEADERS})
target_link_libraries(gba armigo util audio config)

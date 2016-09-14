set(SRCS
    gba.cpp
    arm/arm.cpp
    arm/arm_instr.cpp
    arm/thumb_instr.cpp
    io.cpp
    dma.cpp
    timer.cpp
    memory.cpp
    backup/flash.cpp
    backup/sram.cpp
    video/video.cpp
    video/renderers/text_mode.cpp
    video/renderers/rotate_scale.cpp
    video/renderers/sprite.cpp
    audio/audio.cpp
    audio/quadchannel.cpp
    audio/wavechannel.cpp
)

set(HEADERS
    gba.h
    arm/arm.h
    io.h
    interrupt.h
    memory.h
    backup/backup.h
    backup/flash.h
    backup/sram.h
    video/video.h
    video/video_extra.h
    audio/audio.h
    audio/fifo.h
    audio/quadchannel.h
    audio/wavechannel.h
)

add_library(gba STATIC ${SRCS} ${HEADERS})
target_link_libraries(gba common audio config)

set(SRCS
    gba.cpp
    arm/arm.cpp
    arm/arm_instr.cpp
    arm/thumb_instr.cpp
    memory.cpp
    dma.cpp
    timer.cpp
    video/video.cpp
    backup/flash.cpp
    backup/sram.cpp
    video/soft_composer.cpp
    audio/audio.cpp
    audio/quadchannel.cpp
    audio/wavechannel.cpp
)

set(HEADERS
    gba.h
    arm/arm.h
    iodef.h
    interrupt.h
    memory.h
    video/video.h
    backup/backup.h
    backup/flash.h
    backup/sram.h
    video/composer.h
    video/soft_composer.h
    video/video_extra.h
    audio/audio.h
    audio/fifo.h
    audio/quadchannel.h
    audio/wavechannel.h
)

add_library(gba STATIC ${SRCS} ${HEADERS})
target_link_libraries(gba common audio config)

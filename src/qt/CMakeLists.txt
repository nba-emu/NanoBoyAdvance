set(CMAKE_AUTOMOC ON)

set(SRCS
    main.cpp
    mainwindow.cpp
    screen.cpp
)

set(HEADERS
    mainwindow.h
    screen.h
)

if (WIN32)
    configure_file (
        "${PROJECT_SOURCE_DIR}/res/app.rc.in"
        "${PROJECT_BINARY_DIR}/res/app.rc"
        )
    list(APPEND RESOURCES "${PROJECT_BINARY_DIR}/res/app.rc")
    if (MINGW)
        set(CMAKE_RC_COMPILER_INIT windres)
        enable_language(RC)
        set(CMAKE_RC_COMPILE_OBJECT
            "<CMAKE_RC_COMPILER> -i <SOURCE> -o <OBJECT>")
    endif()
endif()

add_executable(NanoboyAdvance ${SRCS} ${HEADERS} ${RESOURCES})
target_link_libraries(NanoboyAdvance gba)
target_link_libraries(NanoboyAdvance ${QT_DEPS} ${OPENGL_gl_LIBRARY})
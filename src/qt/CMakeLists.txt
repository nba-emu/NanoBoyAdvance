set(CMAKE_AUTOMOC ON)

set(SRCS
    main.cpp
    screen.cpp
    mainwindow.cpp
    settings.cpp
)

set(HEADERS
    screen.hpp
    mainwindow.hpp
    settings.hpp
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

# Create output executable
add_executable(NanoboyAdvance ${SRCS} ${HEADERS} ${RESOURCES})

# Link NanoBoyAdvance Core, Qt, OpenGL and SDL2
target_link_libraries(NanoboyAdvance core)
target_link_libraries(NanoboyAdvance ${QT_DEPS} ${OPENGL_gl_LIBRARY})
target_link_libraries(NanoboyAdvance ${SDL2_LIBRARY})

file(COPY "${PROJECT_SOURCE_DIR}/config.ini" DESTINATION "${PROJECT_BINARY_DIR}/src/qt/")

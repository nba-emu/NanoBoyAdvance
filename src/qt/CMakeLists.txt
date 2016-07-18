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

add_executable(nanoboyadvance_qt ${SRCS} ${HEADERS})
target_link_libraries(nanoboyadvance_qt gba)
target_link_libraries(nanoboyadvance_qt ${QT_DEPS} ${OPENGL_gl_LIBRARY} ${PNG_LIBRARY})
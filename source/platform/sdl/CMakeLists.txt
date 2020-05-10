set(SOURCES
    main.cpp
)

set(HEADERS
    device/audio_device.hpp
)

include(FindSDL2 REQUIRED)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

add_executable(NanoboyAdvance ${SOURCES} ${HEADERS})
target_link_libraries(NanoboyAdvance nba ${SDL2_LIBRARY} ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES})

file(COPY "${PROJECT_SOURCE_DIR}/config.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/keymap.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(CopyShaders ALL
  COMMAND ${CMAKE_COMMAND} -DSRCDIR=${PROJECT_SOURCE_DIR}/shader
          -DDSTDIR=${PROJECT_BINARY_DIR}/source/platform/sdl/shader
          -P ${PROJECT_SOURCE_DIR}/shader.cmake
  COMMENT "Copy GLSL shaders to project build directory."
)
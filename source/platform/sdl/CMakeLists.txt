set(SOURCES
    main.cpp
)

set(HEADERS
    device/audio_device.hpp
)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

add_executable(NanoboyAdvance ${SOURCES} ${HEADERS})
target_link_libraries(NanoboyAdvance nba ${SDL2_LIBRARIES} ${OPENGL_gl_LIBRARY})
if (APPLE)
  target_link_libraries(NanoboyAdvance GLEW::glew)
else()
  target_link_libraries(NanoboyAdvance ${GLEW_LIBRARIES})
endif()

file(COPY "${PROJECT_SOURCE_DIR}/resources/config.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${PROJECT_SOURCE_DIR}/resources/shader/keymap.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(CopyShaders ALL
  COMMAND ${CMAKE_COMMAND} -DSRCDIR=${PROJECT_SOURCE_DIR}/resources/shader
          -DDSTDIR=${PROJECT_BINARY_DIR}/source/platform/sdl/shader
          -P ${PROJECT_SOURCE_DIR}/resources/shader.cmake
  COMMENT "Copy GLSL shaders to project build directory."
)

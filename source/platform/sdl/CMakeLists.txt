set(SOURCES
    main.cpp
)

set(HEADERS
    device/audio_device.hpp
)

include(FindSDL2)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(NanoboyAdvance ${SOURCES} ${HEADERS})
target_include_directories(NanoboyAdvance PRIVATE ${SDL2_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS})
target_link_libraries(NanoboyAdvance nba ${SDL2_LIBRARY} OpenGL::GL)
if (APPLE)
  target_link_libraries(NanoboyAdvance GLEW::glew)
else()
  target_link_libraries(NanoboyAdvance ${GLEW_LIBRARIES})
endif()

file(COPY "${PROJECT_SOURCE_DIR}/resource/config.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${PROJECT_SOURCE_DIR}/resource/keymap.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(CopyShaders ALL
  COMMAND ${CMAKE_COMMAND} -DSRCDIR=${PROJECT_SOURCE_DIR}/resource/shader
          -DDSTDIR=${PROJECT_BINARY_DIR}/source/platform/sdl/shader
          -P ${PROJECT_SOURCE_DIR}/resource/shader.cmake
  COMMENT "Copy GLSL shaders to project build directory."
)

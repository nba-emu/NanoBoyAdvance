project(NanoBoyAdvance-SDL2 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeModules)

set(SOURCES
  main.cpp
)

set(HEADERS
)

find_package(OpenGL REQUIRED)
set(GLEW_VERBOSE ON)
find_package(GLEW REQUIRED)

add_executable(NanoBoyAdvance-SDL2 ${SOURCES} ${HEADERS})
target_include_directories(NanoBoyAdvance-SDL2 PRIVATE ${GLEW_INCLUDE_DIRS})
target_link_libraries(NanoBoyAdvance-SDL2 platform-core OpenGL::GL GLEW::GLEW)
set_target_properties(NanoBoyAdvance-SDL2 PROPERTIES OUTPUT_NAME "NanoBoyAdvance")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resource/config.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resource/keymap.toml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(CopyShaders ALL
  COMMAND ${CMAKE_COMMAND} -DSRCDIR=${CMAKE_CURRENT_SOURCE_DIR}/resource/shader
          -DDSTDIR=${CMAKE_CURRENT_BINARY_DIR}/shader
          -P ${CMAKE_CURRENT_SOURCE_DIR}/resource/shader.cmake
  COMMENT "Copy GLSL shaders to project build directory."
)
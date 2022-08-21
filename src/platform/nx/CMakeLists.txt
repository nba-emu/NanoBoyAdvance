project(NanoBoyAdvance-NX CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## SwitchTools provide various helper functions for building Switch file formats.
#include(SwitchTools)
## Find libnx, which is required by our application.
#find_package(LibNX REQUIRED)

find_package(SDL2 REQUIRED)
# TODO: find GLAD?
find_package(OpenGL REQUIRED)

set(SOURCES
  src/main.cpp
)

set(HEADERS
)

add_executable(NanoBoyAdvance-NX ${SOURCES} ${HEADERS})

target_link_libraries(NanoBoyAdvance-NX PRIVATE platform-core ${NX_STANDARD_LIBRARIES} SDL2::SDL2 OpenGL::GL glad)
target_include_directories(NanoBoyAdvance-NX PRIVATE ${SDL2_INCLUDE_DIR})

# TODO: set NRO metadata such as app name, author and version
nx_create_nro(NanoBoyAdvance-NX)
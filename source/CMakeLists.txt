include_directories(.)

option(PLATFORM_QT "Enable Qt5 frontend" ON)
option(PLATFORM_SDL "Enable SDL2 frontend" ON)

add_subdirectory(core)

if (PLATFORM_QT)
  add_subdirectory("platform/qt")
endif()

if (PLATFORM_SDL)
  add_subdirectory("platform/sdl")
endif()
set(toml11_BUILD_TEST OFF CACHE BOOL "Build toml tests" FORCE)

set(CMAKE_SKIP_INSTALL_RULES YES)

add_subdirectory(fmtlib)
add_subdirectory(toml11)

# TODO: evaluate what values for USE_SYSTEM_* make sense on Linux and macOS:
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(ENABLE_LZ7 ON CACHE INTERNAL "")
set(USE_SYSTEM_BZ2 OFF CACHE INTERNAL "")
set(USE_SYSTEM_LZMA OFF CACHE INTERNAL "")
set(USE_SYSTEM_ZLIB OFF CACHE INTERNAL "")
add_subdirectory(unarr)

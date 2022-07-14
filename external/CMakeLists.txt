set(toml11_BUILD_TEST OFF CACHE BOOL "Build toml tests" FORCE)

add_subdirectory(fmtlib)
add_subdirectory(toml11)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(ENABLE_LZ7 ON CACHE INTERNAL "")
add_subdirectory(unarr)

# CMake toolchain file for Dreamcast cross-compilation via DreamSDK / KallistiOS.
#
# Usage:
#   cmake -B build-dc -G Ninja \
#     -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-dreamcast.cmake \
#     -DPLATFORM_DREAMCAST=ON \
#     -DCMAKE_BUILD_TYPE=Release
#
# Prerequisites:
#   - DreamSDK installed (https://www.dreamsdk.org/) with KallistiOS environment
#   - KOS_BASE, KOS_CC_BASE environment variables set (sourced from environ.sh)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR sh4)
set(CMAKE_CROSSCOMPILING TRUE)

# DreamSDK / KallistiOS paths from environment
if(NOT DEFINED ENV{KOS_BASE})
  message(FATAL_ERROR "KOS_BASE environment variable is not set. Source your KallistiOS environ.sh first.")
endif()

if(NOT DEFINED ENV{KOS_CC_BASE})
  message(FATAL_ERROR "KOS_CC_BASE environment variable is not set. Source your KallistiOS environ.sh first.")
endif()

set(KOS_BASE    $ENV{KOS_BASE})
set(KOS_CC_BASE $ENV{KOS_CC_BASE})
set(KOS_ARCH    "dreamcast")
set(KOS_SUBARCH "pristine")

# Cross-compiler
set(CMAKE_C_COMPILER   "${KOS_CC_BASE}/bin/sh-elf-gcc.exe")
set(CMAKE_CXX_COMPILER "${KOS_CC_BASE}/bin/sh-elf-g++.exe")
set(CMAKE_AR           "${KOS_CC_BASE}/bin/sh-elf-ar.exe" CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB       "${KOS_CC_BASE}/bin/sh-elf-ranlib.exe" CACHE FILEPATH "Ranlib")

# Don't try to run test executables on the host
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# KallistiOS include and library paths
set(KOS_INCS
  "${KOS_BASE}/include"
  "${KOS_BASE}/kernel/arch/${KOS_ARCH}/include"
  "${KOS_BASE}/addons/include"
)

set(KOS_LIBS
  "${KOS_BASE}/lib/${KOS_ARCH}"
  "${KOS_BASE}/addons/lib/${KOS_ARCH}"
)

# Set system include/library paths
include_directories(SYSTEM ${KOS_INCS})
link_directories(${KOS_LIBS})

# KOS compiler/linker flags
set(KOS_CFLAGS "-ml -m4-single-only -D__DREAMCAST__ -D_arch_dreamcast -DPLATFORM_DREAMCAST=1")
set(KOS_CXXFLAGS "${KOS_CFLAGS} -std=c++20")

set(CMAKE_C_FLAGS_INIT   "${KOS_CFLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${KOS_CXXFLAGS}")

set(CMAKE_EXE_LINKER_FLAGS_INIT
  "-ml -m4-single-only -Wl,-Ttext=0x8c010000 -T${KOS_BASE}/utils/ldscripts/shlelf.xc -nostdlib"
)

set(KOS_LINK_LIBS
  "-Wl,--start-group -lkallisti -lstdc++ -lm -lc -lgcc -Wl,--end-group"
)

set(CMAKE_C_STANDARD_LIBRARIES   "${KOS_LINK_LIBS}")
set(CMAKE_CXX_STANDARD_LIBRARIES "${KOS_LINK_LIBS}")

# Where to search for packages/libraries
set(CMAKE_FIND_ROOT_PATH "${KOS_BASE}" "${KOS_CC_BASE}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

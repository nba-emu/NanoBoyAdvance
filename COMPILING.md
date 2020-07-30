NanoboyAdvance can be compiled on Windows, Linux and macOS (FreeBSD should work, but are not tested).
A modern C++17-capable compiler such as Clang/Clang-CL or G++ is mandatory.
MSVC is not supported and most definitely doesn't work in the default configuration (stack overflow while parsing templated code).

### Dependencies

There are a few dependencies that you need to get:
- CMake
- SDL2 development library
- GLEW

On Arch Linux:\
`pacman -S cmake sdl2 glew`

On an Ubuntu or Debian derived system:\
`apt install cmake libsdl2-dev libglew-dev`

On macOS:
- install macOS 10.15 SDK for OpenGL support
- SDL2: `brew install sdl2`
- GLEW: `brew install glew`

### Clone Git repository

Make sure to clone the repository with its submodules:
```
git clone --recurse-submodules https://github.com/fleroviux/NanoboyAdvance.git
```

### Setup CMake build environment

Setup the CMake build in a folder of your choice.

#### Linux
```
cd /somewhere/on/your/system/NanoboyAdvance
mkdir build
cd build
cmake ..
```
A final `make` then compiles the emulator.
The compiled executables then can be found in `build/source/platform/`.

#### macOS
As a workaround you may need to manually expose GLEW headers:
```bash
export CPPFLAGS="$CPPFLAGS -I/usr/local/opt/glew/include"
```
Otherwise the build process should be identical to Linux.

#### Windows
Setup [vcpkg](https://github.com/microsoft/vcpkg) and install the required libraries.
```
git clone https://github.com/microsoft/vcpkg
cd vcpkg
bootstrap-vcpkg.bat
vcpkg install sdl2
vcpkg install glew
```

Generate the Visual Studio solution with CMake.
```
cd path/to/NanoboyAdvance
mkdir build
cd build
set VCPKG_ROOT=path/to/vcpkg
cmake -T clangcl ..
```

Build the Visual Studio solution. It can also be done via the command line.
```
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
msbuild NanoboyAdvance.sln
```

#### Miscellaneous

If you get an error regarding to `libc++fs` not being found, try commenting out `target_link_libraries(nba stdc++fs)` in `source/CMakeLists.txt`.

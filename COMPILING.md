NanoboyAdvance can be compiled on Windows, Linux and macOS.

### Prerequisites

- C++17-capable compiler such as Clang or G++
  - MSVC is unlikely to work, it doesn't work in the default configuration.
- CMake 3.2 or higher
- OpenGL (usually provided by the operating system)
- SDL2 library
- GLEW library

### Source Code

Clone the Git repository and checkout the submodules:  

```bash
git clone https://github.com/fleroviux/NanoboyAdvance.git
git submodule update --init
```

### Unix Build (Linux, macOS)

#### 1. Install dependencies

The way that you install the dependencies will vary depending on the distribution you use.  
Typically you'll have to invoke the install command of some package manager.  
Here is a list of commands for popular distributions and macOS:

##### Arch Linux

```bash
pacman -S cmake sdl2 glew
```

##### Ubuntu or other Debian derived

```bash
apt install cmake libsdl2-dev libglew-dev
```

##### macOS

Get [Brew](https://brew.sh/) and run:
``` bash
brew install cmake sdl2 glew
```

You may need to manually expose GLEW header:
```bash
export CPPFLAGS="$CPPFLAGS -I/usr/local/opt/glew/include"
```

#### 2. Setup CMake build directory

```
cd /somewhere/on/your/system/NanoboyAdvance
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

NOTE: the location and name of the `build` directory is arbitrary.

#### 3. Compile

Just run make:
```
make
```
or to use multiple processor cores:
```
make -jNUMBER_OF_CORES
```
Binaries will be output to `build/source/platform/`

### Windows Mingw-w64 (GCC)

This guide uses [MSYS2](https://www.msys2.org/) to install Mingw-w64 and other dependencies.

#### 1. Install dependencies

In your MSYS2 command line run:
```bash
pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2 mingw-w64-x86_64-glew
```

#### 2. Setup CMake build directory

```bash
cd path/to/NanoboyAdvance
mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
```
NOTE: the location and name of the `build` directory is arbitrary.

#### 3. Compile

Just run make:
```
make
```
or to use multiple processor cores:
```
make -jNUMBER_OF_CORES
```
Binaries will be output to `build/source/platform/`

### Windows Visual Studio (Clang)

This guide uses Visual Studio 2017 (or newer) and [VCPKG](https://github.com/microsoft/vcpkg#quick-start-windows).  

#### 1. Install dependencies

For older Visual Studio versions (before Visual Studio 2019 16.1 Preview 2) you will have to install [Clang](https://devblogs.microsoft.com/cppblog/clang-llvm-support-in-visual-studio/).

##### 32-bit x86 dependencies

```cmd
vcpkg install sdl2
vcpkg install glew
```

##### 64-bit x64 dependencies

```cmd
vcpkg install sdl2:x64-windows
vcpkg install glew:x64-windows
```

#### 2. Setup CMake build directory

Generate the Visual Studio solution with CMake.
```
cd path/to/NanoboyAdvance
mkdir build
cd build
set VCPKG_ROOT=path/to/vcpkg
cmake -T clangcl ..
```

#### 3. Compile

Build the generated Visual Studio solution. It can also be done via the command line.
```
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
msbuild NanoboyAdvance.sln
```


NanoBoyAdvance can be compiled on Windows, Linux, and macOS.

### Prerequisites

- Clang or GCC with C++17 support
- CMake 3.11 or higher
- OpenGL (usually provided by the operating system)
- SDL2 library
- GLEW library
- Qt5 library

### Source Code

Clone the Git repository:

```bash
git clone https://github.com/nba-emu/NanoBoyAdvance.git
```

### Unix Build (Linux, macOS)

#### 1. Install dependencies

The way that you install the dependencies will vary depending on the distribution you use.  
Typically you'll have to invoke the install command of some package manager.  
Here is a list of commands for popular distributions and macOS:

##### Arch Linux

```bash
pacman -S cmake sdl2 glew qt5-base
```

##### Ubuntu or other Debian-derived distribution

```bash
apt install cmake libsdl2-dev libglew-dev qtbase5-dev libqt5opengl5-dev
```

##### macOS

Get [Brew](https://brew.sh/) and run:

``` bash
brew install cmake sdl2 glew qt@5
```

##### FreeBSD

```bash
su
pkg install cmake git sdl2 glew qt5 qt5-opengl
```

#### 2. Setup CMake build directory

```
cd /somewhere/on/your/system/NanoBoyAdvance
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

NOTE: the location and name of the `build` directory is arbitrary.

#### 3. Compile

Run CMake build command:

```
cmake --build build
```

Append `-j $(nproc)` to use all processor cores for compilation.

Binaries will be output to `build/bin/`.

### Windows Mingw-w64 (GCC)

This guide uses [MSYS2](https://www.msys2.org/) to install Mingw-w64 and other dependencies.

#### 1. Install dependencies

In your MSYS2 command line, run:

```bash
pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2 mingw-w64-x86_64-glew mingw-w64-x86_64-qt5-static
```

#### 2. Setup CMake build directory

```bash
cd path/to/NanoBoyAdvance
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
```

NOTE: the location and name of the `build` directory is arbitrary.

#### 3. Compile

Run CMake build command:

```
cmake --build build
```

Append `-j $(nproc)` to use all processor cores for compilation.

Binaries will be output to `build/bin/`.

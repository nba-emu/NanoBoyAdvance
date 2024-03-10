NanoBoyAdvance can be compiled on Windows, Linux, and macOS.

### Prerequisites

- Clang or GCC with C++17 support
- CMake 3.11 or higher
- Python modules Jinja and (optionally) lxml
- OpenGL (usually provided by the operating system)
- SDL 2 library
- Qt 5 library

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
pacman -S cmake python-jinja python-lxml sdl2 qt5-base
```

##### Ubuntu or other Debian-derived distribution

```bash
apt install cmake python3-jinja2 python3-lxml libsdl2-dev qtbase5-dev libqt5opengl5-dev
```

##### macOS

Get [Brew](https://brew.sh/) and run:

``` bash
brew install cmake python@3 sdl2 qt@5
python3 -m pip install Jinja2
```

##### FreeBSD

```bash
su
pkg install cmake git py39-Jinja2 py39-lxml sdl2 qt5 qt5-opengl
```

#### 2. Setup CMake build directory

##### Linux and FreeBSD

```
cd /somewhere/on/your/system/NanoBoyAdvance
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

NOTE: the location and name of the `build` directory is arbitrary.

##### macOS

```
cd /somewhere/on/your/system/NanoBoyAdvance
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix qt@5)"
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
pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-python-jinja mingw-w64-x86_64-python-lxml mingw-w64-x86_64-SDL2 mingw-w64-x86_64-qt5-static
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

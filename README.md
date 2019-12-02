<h2>NanoboyAdvance</h2>

![logo](media/logo_cropped.png)

![license](https://img.shields.io/github/license/fleroviux/NanoboyAdvance)

NanoboyAdvance is a Nintendo GameBoy Advance (TM) emulator written in C++17.<br>
The goal is to create a modern, accurate GameBoy Advance emulator while also being fast
and avoiding to sacrifice too much code quality.

The emulator implements the core hardware completely and with fairly high accuracy.
Any game that I tested so far goes in-game in NanoboyAdvance, almost all of them without known issues.
Even the games from the `Classic NES` series, which are known to be troublesome to emulate, work properly. 

#### Media

![screenshot1](media/screenshot1.png)

# Compiling

Currently the code is known to be compilable on Linux and Windows.
Use of a recent G++ compiler is recommmended. Other compilers *might* work but have not been tested so far.
Your version of G++ should support C++17.

In order to compile NanoboyAdvance you will need a few dependencies:
- CMake
- SDL2 development files (libsdl2-dev on a Debian-based system)

## Setup

Download or clone the repository to a folder of your choice.
Then open a command prompt, create a build folder, run `cmake` and make to build the emulator.
```bash
cd /your/path/NanoboyAdvance
mkdir build
cd build
cmake ..
make
```
The compiled executable can be found at `build/source/platform/sdl/`.

## Rebuilding

When rebuilding you can just re-run make from the command prompt.
```bash
cd /your/path/NanoboyAdvance/build
make
```

# Running

In order to run NanoboyAdvance you will need a BIOS file.
You can either dump your own or get an open source replacement online (https://github.com/Nebuleon/ReGBA/blob/master/bios/gba_bios.bin).
The BIOS file must be placed as `bios.bin` in the same folder as the executable.

You can then run the emulator:
```bash
./NanoboyAdvance-SDL path_to_your_rom.gba
```

# Acknowledgement

- GBATEK by Martin Korth: a great piece of documentation that made this emulator possible.
- VisualBoyAdvance by Forgotten: inspiring me to pursue emulator programming.

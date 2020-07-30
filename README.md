<h2>NanoboyAdvance</h2>

![logo](media/logo_cropped.png)

![license](https://img.shields.io/github/license/fleroviux/NanoboyAdvance)
[![CodeFactor](https://www.codefactor.io/repository/github/fleroviux/NanoboyAdvance/badge)](https://www.codefactor.io/repository/github/fleroviux/NanoboyAdvance)

NanoboyAdvance is a Nintendo Game Boy Advance (TM) emulator written in C++17.<br>
The goal is to create a minimal, accurate and reasonably efficient Game Boy Advance emulator in modern C++.

The emulator implements the core hardware completely and with high accuracy.
Almost all games can be emulated without any known issues, even the 'Classic NES' titles which abuse a
variety of edge-cases and undefined behaviour.

![screenshot1](media/screenshot1.png)

## Features

- very accurate GBA emulation
- RTC emulation
- high quality audio rendering
- game controller support
- basic GLSL shader support
- lightweight: minimal, configurable SDL2 frontend

## Compiling

See [COMPILING.md](https://github.com/fleroviux/NanoboyAdvance/blob/master/COMPILING.md) in the root directory of this project.

## Running

In order to run NanoboyAdvance you will need a BIOS file.
You can either dump your own or get an open source replacement online (https://github.com/Nebuleon/ReGBA/blob/master/bios/gba_bios.bin).
The BIOS file must be placed as `bios.bin` in the same folder as the executable. However keep in mind, that a replacement BIOS
will not be as accurate as the original one.

## Acknowledgement

- GBATEK by Martin Korth: a great piece of documentation that made this emulator possible.
- Talarubi & Near: The GBA color correction shader (https://byuu.net/video/color-emulation)
- mGBA by endrift: great test suite, also referenced its code for a few details.
- VisualBoyAdvance by Forgotten: inspired me to pursue emulator programming.

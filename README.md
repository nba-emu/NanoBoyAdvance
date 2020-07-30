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

## Running

You'll need a Game Boy Advance BIOS dump or a [replacement BIOS](https://github.com/Nebuleon/ReGBA/blob/master/bios/gba_bios.bin).  
Do note though that a legitimate BIOS will always be more accurate than a replacement.

Place the BIOS file named as `bios.bin` into the same folder as the executable or provide a path via the CLI or [config.toml](https://github.com/fleroviux/NanoboyAdvance/blob/master/resource/config.toml) file.

#### CLI arguments
```
NanoboyAdvance.exe [--bios bios_path] [--force-rtc] [--save-type type] [--fullscreen] [--scale factor] [--resampler type] [--sync-to-audio yes/no] rom_path
```
See [config.toml](https://github.com/fleroviux/NanoboyAdvance/blob/master/resource/config.toml) for more documentation or options.

## Compiling

See [COMPILING.md](https://github.com/fleroviux/NanoboyAdvance/blob/master/COMPILING.md) in the root directory of this project.

## Acknowledgement

- GBATEK by Martin Korth: a great piece of documentation that made this emulator possible.
- Talarubi & Near: GBA color correction algorithm (https://byuu.net/video/color-emulation)
- endrift: for the gba-suite Test-ROM and mGBA which I referenced for a few details.

# Migrated to Codeberg!

Development of the project on GitHub has ceased and will continue over on [Codeberg](https://codeberg.org/nba-emu/NanoBoyAdvance).

# NanoBoyAdvance
[![discord](https://img.shields.io/discord/969218483873251338?logo=discord&label=discord)](https://discord.gg/HDshKwvcj)

NanoBoyAdvance is a cycle-accurate Game Boy Advance emulator.  
It aims to be as accurate as possible, while also offering enhancements such as improved audio quality.  

## Features
- Very high compatibility and accuracy (see [Accuracy](#accuracy))
- HQ audio mixer (for games which use Nintendo's MusicPlayer2000 sound engine)
- Post-processing options (color correction, xBRZ upscaling and LCD ghosting simulation)
- Save State support (10x save slots available)
- Game controller support (buttons and axises can be remapped)
- Loading ROMs from archives (including ZIP, 7z, tar and more)
- RTC emulation
- Solar Sensor emulation (for example: for Boktai - The Sun is in Your Hand)
- Debug tools: PPU palette, tile, background and sprite viewers

## Running
Download the lastest [release](https://github.com/nba-emu/NanoBoyAdvance/releases).

Upon loading a ROM for the first time you will be prompted to assign the Game Boy Advance BIOS file.  
You can [dump](https://github.com/mgba-emu/bios-dump) it from a real console (accurate) or use an [unofficial BIOS](https://github.com/Nebuleon/ReGBA) (Use `gba_bios.bin` from the `bios` folder.) (less accurate).

## Accuracy
A lot of research and attention to detail has been put into developing this core and making it accurate.

- Cycle-accurate emulation of most components, including: CPU, DMA, timers, PPU and Game Pak prefetch
- Passes all AGS aging cartridge tests (NBA was the first public emulator to achieve this)
- Passes most tests in the [mGBA test suite](https://github.com/mgba-emu/suite)
- Passes [ARMWrestler](https://github.com/destoer/armwrestler-gba-fixed), [gba-suite](https://github.com/jsmolka/gba-tests) and [FuzzARM](https://github.com/DenSinH/FuzzARM) CPU tests
- Very high compatibility, including games that require emulation of peculiar hardware edge-cases

## Compiling
Prerequisites:
- Clang or GCC with C++20 support
- CMake 3.24 or newer (Recommended generator is Ninja)
- SDL 3.0.0 or newer
- Qt 6.0 or newer

| Distribution | Packages | Package Manager | Notes |
|---|---|---|---|
| Arch Linux | `cmake ninja sdl3 qt6-base` | pacman | |
| Debian & Ubuntu | `cmake ninja-build libsdl3-dev qt6-base-dev` | apt | |
| | | | |
| macOS | `cmake ninja sdl3 qt@6` | Homebrew | |
| | | | |
| FreeBSD | `cmake devel/ninja git sdl3 qt6-base` | pkg | Untested on CI. Use at your own risk:tm:. |

You can then configure NanoBoyAdvance:
```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

For Windows, append `-DQt6_DIR=path\to\Qt6-config.cmake` and `-DSDL3_DIR=path\to\SDL3-config.cmake`, with the correct paths given.

Compiling can now be done through `cmake --build build`.

## Acknowledgements
| Individual(s) | Their impact |
|---|---|
| Martin Korth | [GBATEK](http://problemkaputt.de/gbatek.htm), a good piece of hardware documentation |
| [endrift](https://github.com/endrift) | Hardware [research](http://mgba.io/tag/emulation/) and her excellent [test suite](https://github.com/mgba-emu/suite) |
| [destoer](https://github.com/destoer), [Noumi](https://github.com/noumidev), [Sky](https://github.com/skylersaleh) and [Zayd](https://github.com/zaydlang) | Hardware research and tests, countless discussions and being good friends |
| Pokefan531, hunterk | The default GBA color correction algorithm |
| Talarubi, Near | [higan's GBA color correction algorithm](https://archive.ares-emu.net/near.sh/articles/video/color-emulation.html) |
| DeSmuME, Hyllian | xBRZ upscaling code |

## Sister Projects
Our sister projects include **[SkyEmu](https://github.com/skylersaleh/SkyEmu)**, **[Dust](https://github.com/Kelpsyberry/dust)** and **[Panda3DS](https://github.com/wheremyfoodat/panda3DS)**.

## Copyright
NanoBoyAdvance is licensed under the terms of the GNU General Public License (GPL) 3.0 or any later version.
See [LICENSE](LICENSE) for details.

Game Boy Advance is a registered trademark of Nintendo Co., Ltd.

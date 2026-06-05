# NanoBoyAdvance
[![license](https://img.shields.io/github/license/nba-emu/NanoBoyAdvance)](https://github.com/nba-emu/NanoBoyAdvance/blob/master/LICENSE)
[![build](https://img.shields.io/github/actions/workflow/status/nba-emu/NanoBoyAdvance/build.yml?branch=master)](https://github.com/nba-emu/NanoBoyAdvance/actions/workflows/build.yml)
[![discord](https://img.shields.io/discord/969218483873251338?logo=discord&label=discord)](https://discord.gg/4NnUjsf7Am)

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

## Project Scope
This fork of NanoBoyAdvance is a **Dreamcast-only port**. 
- **In Scope:** Retail Dreamcast hardware, common ODE/CD workflows, KallistiOS toolchain builds.
- **Out of Scope:** Desktop builds (Windows, Linux, macOS) or other platforms.

### Feature Matrix (Dreamcast Port)
| Feature | Status | Notes |
|---------|--------|-------|
| Core Emulation | Working | Accurate CPU/PPU emulation |
| Audio Output | Working | snd_stream output; MP2K HLE disabled |
| Video Output | Working | PVR framebuffer |
| Input | Working | Maple controller |
| Save Files | Partial | Per-ROM saves at `/pc/saves`, no VMU support yet |
| Save States | Missing | UI not implemented |
| ROM Browser | Working | Scans `/pc/roms` and `/cd` |

## Running & Compiling
For instructions on how to build and run this port on the Sega Dreamcast, please see **[DREAMCAST.md](DREAMCAST.md)**.

Upon loading a ROM for the first time, the emulator requires a Game Boy Advance BIOS file.  
You can [dump](https://github.com/mgba-emu/bios-dump/tree/master/src) it from a real console (accurate) or use an [unofficial BIOS](https://github.com/Nebuleon/ReGBA/blob/master/bios/gba_bios.bin) (less accurate).

## Accuracy
A lot of research and attention to detail has been put into developing this core and making it accurate.

- Cycle-accurate emulation of most components, including: CPU, DMA, timers, PPU and Game Pak prefetch
- Passes all AGS aging cartridge tests (NBA was the first public emulator to achieve this)
- Passes most tests in the [mGBA test suite](https://github.com/mgba-emu/suite)
- Passes [ARMWrestler](https://github.com/destoer/armwrestler-gba-fixed), [gba-suite](https://github.com/jsmolka/gba-tests) and [FuzzARM](https://github.com/DenSinH/FuzzARM) CPU tests
- Very high compatibility, including games that require emulation of peculiar hardware edge-cases

## Compiling

*Please note: Desktop builds (Linux, macOS, Windows) are no longer supported in this fork.*

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
Our sister projects include **[SkyEmu](https://github.com/skylersaleh/SkyEmu/)**, **[Dust](https://github.com/Kelpsyberry/dust/)** and **[Panda3DS](https://github.com/wheremyfoodat/panda3DS)**.

## Copyright
NanoBoyAdvance is licensed under the terms of the GNU General Public License (GPL) 3.0 or any later version.
See [LICENSE](LICENSE) for details.

Game Boy Advance is a registered trademark of Nintendo Co., Ltd.

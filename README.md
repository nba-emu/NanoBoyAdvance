<h2>NanoBoyAdvance</h2>

![license](https://img.shields.io/github/license/nba-emu/NanoBoyAdvance)
![build](https://img.shields.io/github/actions/workflow/status/nba-emu/NanoBoyAdvance/build.yml?branch=master)

NanoBoyAdvance is a cycle-accurate Game Boy Advance emulator.<br>
It aims to be as accurate as possible, while also offering enhancements such as
improved audio quality.<br>

![screenshot1](docs/screenshot.png)

## Features
- Very high compatibility and accuracy (see [Accuracy](#accuracy))
- HQ audio mixer (for games which use Nintendo's MusicPlayer2000 sound engine)
- Post-processing options (color correction, xBRZ upscaling and LCD ghosting simulation)
- Save State support (10x save slots available)
- Game controller support (buttons and axises can be remapped)
- Loading ROMs from archives (Zip, 7z, Tar and limited RAR[^1] support)
- RTC emulation
- Solar Sensor emulation (for example: for Boktai - The Sun is in Your Hand)

[^1]: RAR 5.0 is currently not supported.

## Running

Download a recent [development build](https://nightly.link/nba-emu/NanoBoyAdvance/workflows/build/master) or the last [stable release](https://github.com/nba-emu/NanoBoyAdvance/releases).

Upon loading a ROM for the first time you will be prompted to assign the Game Boy Advance BIOS file.  
You can [dump](https://github.com/mgba-emu/bios-dump/tree/master/src) it from a real console (accurate) or use an [unofficial BIOS](https://github.com/Nebuleon/ReGBA/blob/master/bios/gba_bios.bin) (less accurate).

## Accuracy

A lot of research and attention to detail has been put into developing this core and making it accurate.

- Cycle-accurate emulation of most components, including: CPU, DMA, timers, PPU and Game Pak prefetch
- Passes all AGS aging cartridge tests (NBA was the first public emulator to achieve this)
- Passes most tests in the [mGBA test suite](https://github.com/mgba-emu/suite) (see [mGBA suite comparison](docs/ACCURACY.md#mGBA-suite-comparison) for more details)
- Passes [ARMWrestler](https://github.com/destoer/armwrestler-gba-fixed), [gba-suite](https://github.com/jsmolka/gba-tests) and [FuzzARM](https://github.com/DenSinH/FuzzARM) CPU tests
- Very high compatibility, including games that require emulation of peculiar hardware edge-cases (see [Game compatibility](docs/ACCURACY.md#Game-compatibility))

## Compiling

See [COMPILING.md](docs/COMPILING.md) in the `docs` folder.

## Credit

- Martin Korth: for [GBATEK](http://problemkaputt.de/gbatek.htm), a good piece of hardware documentation.
- [endrift](https://github.com/endrift): for prior [research](http://mgba.io/tag/emulation/) and [hardware tests](https://github.com/mgba-emu/suite).
- [destoer](https://github.com/destoer): for contributing research, tests and insightful discussions.
- [LadyStarbreeze](https://github.com/LadyStarbreeze): for contributing research, tests and insightful discussions.
- Pokefan531 and hunterk: for the default GBA color correction algorithm
- Talarubi and Near: for [higan's GBA color correction algorithm](https://near.sh/articles/video/color-emulation)
- DeSmuME team and Hyllian: xBRZ upscaling code

## Copyright

NanoBoyAdvance is Copyright © 2015 - 2022 fleroviux.<br>
It is licensed under the terms of the GNU General Public License (GPL) 3.0 or any later version. See [LICENSE](LICENSE) for details.

Game Boy Advance is a registered trademark of Nintendo Co., Ltd.

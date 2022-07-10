<h2>NanoBoyAdvance</h2>

![license](https://img.shields.io/github/license/nba-emu/NanoBoyAdvance)
![build](https://img.shields.io/github/workflow/status/nba-emu/NanoBoyAdvance/Build/master)

NanoBoyAdvance is a highly accurate Game Boy Advance emulator.<br>
It aims to be the most accurate Game Boy Advance emulator, while also offering enhancements such as
improved audio quality.

![screenshot1](docs/screenshot.png)

## Features
- Very high compatibility and accuracy (see [Accuracy](#accuracy))
- HQ audio mixer (for games which use Nintendo's MusicPlayer2000 sound engine)
- Post-processing options (color correction, xBRZ upscaling and LCD ghosting simulation)
- Game controller support (buttons and axises can be remapped)
- RTC emulation

## Running

Download a recent [development build](https://nightly.link/nba-emu/NanoBoyAdvance/workflows/build/master) or the last [stable release](https://github.com/nba-emu/NanoBoyAdvance/releases).

Upon loading a ROM for the first time you will be prompted to assign the Game Boy Advance BIOS file.  
You can [dump](https://github.com/mgba-emu/bios-dump/tree/master/src) it from a real console (accurate) or use an [unofficial BIOS](https://github.com/Nebuleon/ReGBA/blob/master/bios/gba_bios.bin) (less accurate).

## Accuracy

A lot of attention to detail has been put into developing this core and making it accurate.
Its CPU and timing emulation is more accurate than other software emulators right now. 

- Cycle-accurate emulation of the CPU, DMA, timers and Game Pak prefetch buffer
- Passes all AGS aging cartridge tests (NBA was the first public emulator to achieve this)
- Passes most tests in the [mGBA test suite](https://github.com/mgba-emu/suite) (see [ACCURACY.md](docs/ACCURACY.md) for more details)
- Passes [ARMWrestler](https://github.com/destoer/armwrestler-gba-fixed), [gba-suite](https://github.com/jsmolka/gba-tests) and [FuzzARM](https://github.com/DenSinH/FuzzARM) CPU tests
- Runs many difficult to emulate games without game-specific hacks, for example:
   - Hello Kitty Collection: Miracle Fashion Maker
   - Hamtaro Rainbow Rescue (with Spanish or German language)
   - Classic NES series
   - Golden Sun
   - James Pond - Codename Robocod
   - Phantasy Star Collection
   
Cycle-accurate PPU emulation is an active topic of research and will be implemented, once the timing has been understood and documented well enough.

## Compiling

See [COMPILING.md](https://github.com/fleroviux/NanoboyAdvance/blob/master/docs/COMPILING.md) in the `docs` folder.

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

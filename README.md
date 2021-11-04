<h2>NanoBoyAdvance</h2>

![license](https://img.shields.io/github/license/nba-emu/NanoBoyAdvance)
![build](https://img.shields.io/github/workflow/status/nba-emu/NanoBoyAdvance/Build/master)

NanoBoyAdvance is a Game Boy Advance emulator with a focus on high accuracy.<br>

![screenshot1](docs/screenshot.png)

## Features
- highly-accurate GBA emulation
- HQ audio mixer (for titles which use the Sappy/MP2K/M4A sound engine)
- RTC emulation
- game controller support
- GLSL shader support

## Accuracy
- close to cycle-accurate CPU and DMA emulation
- hardware is updated at the bus cycle (sub-instruction) level
- very accurate ARM emulation
- PPU is still scanline based for now

See [ACCURACY.md](docs/ACCURACY.md) for more information on which tests NanoBoyAdvance passes and fails.

## Running

Download NanoBoyAdvance from the [releases page](https://github.com/nba-emu/NanoBoyAdvance/releases) (or get a [nightly build](https://nightly.link/nba-emu/NanoBoyAdvance/workflows/build/master)).

Upon loading a ROM for the first time you will be prompted to assign the Game Boy Advance BIOS file.  
You can [dump](https://github.com/mgba-emu/bios-dump/tree/master/src) it from a real console (accurate) or use an [unofficial BIOS](https://github.com/Nebuleon/ReGBA/blob/master/bios/gba_bios.bin) (less accurate).

## Compiling

See [COMPILING.md](https://github.com/fleroviux/NanoboyAdvance/blob/master/docs/COMPILING.md) in the `docs` folder.

## Credit

- Martin Korth: for [GBATEK](http://problemkaputt.de/gbatek.htm), a good piece of hardware documentation.
- [endrift](https://github.com/endrift): for prior [research](http://mgba.io/tag/emulation/) and [hardware tests](https://github.com/mgba-emu/suite).
- [destoer](https://github.com/destoer): for contributing research, tests and insightful discussions.
- [LadyStarbreeze](https://github.com/LadyStarbreeze): for contributing research, tests and insightful discussions.
- Talarubi, Near: for the default [GBA color correction algorithm](https://near.sh/articles/video/color-emulation)

## Copyright

NanoBoyAdvance is Copyright © 2015 - 2021 fleroviux.<br>
It is licensed under the terms of the GNU General Public License (GPL) 3.0 or any later version. See [LICENSE](LICENSE) for details.

Game Boy Advance is a registered trademark of Nintendo Co., Ltd.

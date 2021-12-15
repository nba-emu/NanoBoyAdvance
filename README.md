<h2>NanoBoyAdvance</h2>

![license](https://img.shields.io/github/license/nba-emu/NanoBoyAdvance)
![build](https://img.shields.io/github/workflow/status/nba-emu/NanoBoyAdvance/Build/master)

NanoBoyAdvance is a Game Boy Advance emulator focused on accuracy.<br>

![screenshot1](docs/screenshot.png)

## Features
- Very high compatibility (see [Accuracy](#accuracy))
- HQ audio mixer (for games which use Nintendo's MusicPlayer2000 sound engine)
- Post-processing options:
    - Color correction
    - Texture filtering (nearest, linear, xBRZ)
    - LCD ghosting simulation
- RTC emulation
- Game controller support

## Running

Download NanoBoyAdvance from the [releases page](https://github.com/nba-emu/NanoBoyAdvance/releases) (or get a [nightly build](https://nightly.link/nba-emu/NanoBoyAdvance/workflows/build/master)).

Upon loading a ROM for the first time you will be prompted to assign the Game Boy Advance BIOS file.  
You can [dump](https://github.com/mgba-emu/bios-dump/tree/master/src) it from a real console (accurate) or use an [unofficial BIOS](https://github.com/Nebuleon/ReGBA/blob/master/bios/gba_bios.bin) (less accurate).

## Accuracy

A lot of attention to detail has been put into developing this core and making it accurate.
Its CPU and timing emulation is more accurate than other software emulators right now. 

- CPU and DMA is emulated with near cycle-accuracy
- ARM emulation handles nearly all known edge-cases
- Runs many difficult to emulate games without game-specific hacks, for example:
   - Hello Kitty Collection: Miracle Fashion Maker
   - Hamtaro Rainbow Rescue (with Spanish or German language)
   - James Pond - Codename Robocod
   - Phantasy Star Collection
   - Golden Sun
   - Classic NES series
- Passes all AGS aging cartridge tests (NBA was the first public emulator to achieve this)
- See [ACCURACY.md](docs/ACCURACY.md) for a select list of tests that NBA passes

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

NanoBoyAdvance is Copyright © 2015 - 2021 fleroviux.<br>
It is licensed under the terms of the GNU General Public License (GPL) 3.0 or any later version. See [LICENSE](LICENSE) for details.

Game Boy Advance is a registered trademark of Nintendo Co., Ltd.

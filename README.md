<h2>NanoBoyAdvance</h2>

![license](https://img.shields.io/github/license/fleroviux/NanoboyAdvance)
[![CodeFactor](https://www.codefactor.io/repository/github/fleroviux/NanoboyAdvance/badge)](https://www.codefactor.io/repository/github/fleroviux/NanoboyAdvance)

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

## Running

Get stable versions on the [releases page](https://github.com/fleroviux/NanoboyAdvance/releases).
Or alternatively get the latest [nightly build ](https://nightly.link/fleroviux/NanoBoyAdvance/workflows/build/master).

A legitimate Game Boy Advance BIOS dump or a [replacement BIOS](https://github.com/Nebuleon/ReGBA/blob/master/bios/gba_bios.bin) is required.  
Do note though that the replacement BIOS is less accurate.

Place your BIOS file named as `bios.bin` into the same folder as the executable or provide a path via the CLI or [config.toml](https://github.com/fleroviux/NanoBoyAdvance/blob/master/src/platform/sdl/resource/config.toml)
#### CLI arguments
```
NanoboyAdvance.exe [--bios bios_path] [--force-rtc] [--save-type type] [--fullscreen] [--scale factor] [--resampler type] [--sync-to-audio yes/no] rom_path
```
See [config.toml](https://github.com/fleroviux/NanoBoyAdvance/blob/master/src/platform/sdl/resource/config.toml) for more documentation or options.

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
It is licensed under the terms of the GNU General Public License (GPL) or any later version. See [LICENSE](LICENSE) for details.

Game Boy Advance is registered trademark of Nintendo Co., Ltd.

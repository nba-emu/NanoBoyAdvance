# NanoboyAdvance

NanoboyAdvance is a modern Game Boy Advance emulator written in C++ with performance, platform independency and reasonable accuracy in mind. This emulator is actively being developed on to reach the goals we're aiming for. However currently we still
have some work to do until a first release targeted at users. NanoboyAdvance will eventually target a great range of hardware and devices and will make use of efficient code and GPU-based hardware acceleration for the best emulation experience on low power devices while maintaining a clean and understandable code base.

## Media

<img src="https://puu.sh/qlnpt/3c6e1a056d.png" alt="Pokemon Fire Red">

## Status:

ARM: 99%

Memory: 99%

Video: ~92%

Audio: 90%

Flash Support: yes

SRAM Support: yes

EEPROM Support: no

## Compiling

```
mkdir build
cd build
cmake %FLAGS% %SOURCE_PATH%
make
```
Replace `%FLAGS%` with flags (if any) with flags that shall be passed to `cmake`.

Replace `%SOURCE_PATH%` with the path where you extracted the source / cloned the repository.

### CMake Options
`DEBUG`: Enable compilation for debugging.

`SDL2`: Enables SDL2 frontend

`QT_GUI`: Enables Qt5 frontend

`PROFILE`: Compiles for profiling with GNU gprof.

## Credits

- endrift(mGBA): I adapted their banking algorithm in my new ARM interpreter.
- nocash/Martin Korth: for the creation of GBATEK
- VisualBoyAdvance Team: basically rocked my childhood and inspired me to write my own emulator.

## Contributing

Are you a developer and think you an help us out? We're always looking for motivated and skilled developers who want to support us. If you're interested in helping us out, please write me an e-mail to `fredericmeyer1337@yahoo.de`. We don't have an IRC channel yet.

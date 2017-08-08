# NanoboyAdvance

NanoboyAdvance is an experimental GameBoyAdvance (TM) emulator written in C++11. Its core is designed to be 100% platform-independent and to run games at good speed even on older hardware such as netbooks. Ports to mobile devices and handhelds are also planned, but might need more optimization or a JIT compiler though.

## Status:

The emulator is at an early stage but it can already boot a high amount of commercial games. Audio and many Graphics features are missing and timing is very inaccurate (doesn't distinguish between N/S cycles and no prefetch emulation).

## Compiling

-IMPORTANT: You must download Qt at https://www.qt.io/download-open-source/
-IMPORTANT: You must also download SDL at https://www.libsdl.org/download-2.0.php
-You must add these to the PATH, or add them to the cache if you are using CMake GUI.
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

`PROFILE`: Compiles for profiling with GNU gprof.

## Credits

- endrift(mGBA): I adapted their register banking algorithm in my refined ARM interpreter.
- fmtlib: format-library that is included in the source. https://github.com/fmtlib/fmt
- nocash/Martin Korth: for the creation of GBATEK
- VisualBoyAdvance Team: basically rocked my childhood and inspired me to write my own emulator.


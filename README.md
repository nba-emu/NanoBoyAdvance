# NanoboyAdvance

NanoboyAdvance is an experimental GameBoyAdvance (TM) emulator written in C++11. Its core is designed to be 100% platform-independent and to run games at good speed even on older hardware such as netbooks. Ports to mobile devices and handhelds are also planned, but might need more optimization or a JIT compiler though.

## Status:

The emulator is at an early stage but it can already boot a high amount of commercial games. Audio and many Graphics features are missing and timing is very inaccurate (doesn't distinguish between N/S cycles and no prefetch emulation).

## Compiling

For Windows users:

-IMPORTANT: You must download SDL2 at https://www.libsdl.org/download-2.0.php  
-IMPORTANT: You must also download Qt at https://www.qt.io/download-open-source/ (if you want to build the Qt frontend)  
-You must add these to the PATH, and you must also add these to your compiler folder with the following procedure: 

1. Download the library version for your specific compiler.
2. copy the /lib folder in the library into your compiler /lib folder
3. copy the /include folder in the library into your compiler /include folder
4. Copy the library DLLs into your compiler /bin folder.

Linux users usually just install the libraries via package manager and should be good.

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
`SDL2`: Enables SDL2 frontend (please note SDL2 is always required though, for audio)  
`QT_GUI`: Enables Qt5 frontend which is more user-friendly than the plain SDL2 build  
`PROFILE`: Compiles for profiling with GNU gprof.  

## Credits

- endrift(mGBA): I adapted their register banking algorithm in my refined ARM interpreter.
- fmtlib: format-library that is included in the source. https://github.com/fmtlib/fmt
- nocash/Martin Korth: for the creation of GBATEK
- VisualBoyAdvance Team: basically rocked my childhood and inspired me to write my own emulator.


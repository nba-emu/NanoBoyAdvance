# atom

![license](https://img.shields.io/github/license/fleroviux/atom)

atom is a collection of C++ libraries that I have written for use in my personal projects.

## Motivation

I used to have the same or similar code for things like logging, linear algebra and error handling in multiple projects.
This was not ideal, as improvements made to the code in one project, could not be quickly and easily brought over to the other projects.

atom is an attempt to maintain my core libraries in a single place and with a consistent code style to maximize code reuse and minimize maintenance effort.

## Modules

- Atom Common:
  - Sized integer types (u8, s8, u16, s16, ...)
  - Panic macro with support for a custom panic handler
  - Meta-programming utilities
  - Bitwise arithmetic utilities
  - Parse executable (command line) arguments

- Atom Logger:
  - Logger with multi-sink support
  - Per-logger and per-sink log levels
  - Support for custom sinks
  - Support for sharing sink collections between loggers
  
- Atom Math:
  - Vector2, Vector3, Vector4
  - Matrix4
  - Quaternion
  - Box3 (axis-aligned bounding box)
  - Frustum
  - Plane

## License

atom is licensed under a 0BSD license. See [LICENSE](LICENSE) file for details.

atom uses [{fmt}](https://github.com/fmtlib/fmt/) which is licensed under a modified [MIT license](https://github.com/fmtlib/fmt/blob/master/LICENSE.rst):
```
Copyright (c) 2012 - present, Victor Zverovich

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

--- Optional exception to the license ---

As an exception, if, as a result of your compiling your source code, portions of this Software are embedded into a machine-executable object form of such source code, you may redistribute such embedded portions in such object form without including the above copyright and permission notices.
```

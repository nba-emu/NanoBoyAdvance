# Third Party Dependencies
Vendored libraries that aren't commonly available everywhere and not big enough to warrant using prebuilt versions.

| Library | Version | Additional Notes |
|---|---|---|
| [fmt](https://github.com/fmtlib/fmt) | 12.1.0 | |
| [Glad](https://github.com/Dav1dde/glad/) | 2.0.8 | [Pre-generated](https://gen.glad.sh). |
| [unarr](https://github.com/selmf/unarr) | 1.1.1 | Promoted the minimum CMake version requirement to 3.5, Removed usage of `-undefined error` for Apple Clang. |
| [toml11](https://github.com/ToruNiina/toml11) | 4.4.0 | Patched to remove some deprecated quotation mark operator usage. |

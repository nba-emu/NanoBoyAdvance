# (lib)unarr changelog

## 1.1.1 (2023-10-23)

### Fixed
* Fix heap corruption in rar filters (reported by Radosław Madej from Check Point Research)

### Other
* Update Readme

## 1.1.0 (2023-09-03)

### Added
* libFuzzer target for coverage-guided fuzz testing (Wang Xin-yu (王昕宇))
* Unit testing using CMocka
* Integration tests
* Update lzma SDK to v23.01

### Changed
* Build 7z support by default

### Fixed
* Fix pkg-config when using absolute paths
* Fix bzip2 integration

## 1.1.0.beta1 - 2022-05-01

### Added
* Support building unarr-test using CMake
* Build options for disabling system library usage/detection (bzip2, liblzma, zlib)
* Create CMake config-files for downstream integration
* New ar_entry_get_raw_name function for getting raw zip filenames (usefull for faulty zip archives with non-spec filename encodings)
* Support for Loongson CPUs (Liu Xiang)
* Faster crc32 based on Intel slice-by-8 algorithm

### Changed
* Update LZMA SDK code to version 21.07
* Restore limited support for 7z archive extraction (using an embedded subset of LZMA SDK)
* Convert source to LF line endings
* Increase UNARR_API_VERSION to 110
* Use internal crc32 implementation by default

### Fixed
* Fixed a possible memleak in rar filter code found by clang static analyzer
* Fixed some edge cases that could lead to nullpointer dereferences and/or undefined behavior in rar extraction code
* Fixed out of bonds memmove in zip code
* Fixed memleak when trying to open an invalid 7z file
* Fix some minor problems with BZip2 and liblzma not added correctly to pkg-config
* Fix MinGW build

## 1.0.1 - 2017-11-04
This is a bugfix release.

### Fixed
* Fixed typo in pkg-config.pc.cmake template

## 1.0.0 - 2017-09-22

### Added
* Cmake based build system for library builds
* Support for pkg-config (libunarr.pc)
* Windows compatible export header for DLL builds
* xz utils / libLZMA can be used as decoder for LZMA1 and XZ (LZMA2) compressed
ZIP archives.
* The internal LZMA1 decoder can be replaced with xz utils / libLZMA if present

### Changed
* LZMA SDK code was updated to version 17.01 beta
* 7z extraction support is currently broken due to LZMA SDK api changes.
* Unarr sample application (unarr-test) and its makefile (legacy unarr build system) have been moved to the [test](test) folder

### Fixed
* Various small bugfixes related to compiler warnings

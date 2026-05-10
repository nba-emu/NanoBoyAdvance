|Build|Status|
|---|------|
|Linux|[![Build Status](https://dev.azure.com/licorn0647/licorn/_apis/build/status/selmf.unarr?branchName=master&jobName=Linux)](https://dev.azure.com/licorn0647/licorn/_build/latest?definitionId=2&branchName=master)|
|MacOS|[![Build Status](https://dev.azure.com/licorn0647/licorn/_apis/build/status/selmf.unarr?branchName=master&jobName=MacOS)](https://dev.azure.com/licorn0647/licorn/_build/latest?definitionId=2&branchName=master)|
|Windows|[![Build Status](https://dev.azure.com/licorn0647/licorn/_apis/build/status/selmf.unarr?branchName=master&jobName=Windows)](https://dev.azure.com/licorn0647/licorn/_build/latest?definitionId=2&branchName=master)|

# (lib)unarr

**(lib)unarr** is a decompression library for RAR, TAR, ZIP and 7z* archives.

It was forked from **unarr**, which originated as a port of the RAR extraction
features from The Unarchiver project required for extracting images from comic
book archives. [Zeniko](https://github.com/zeniko/) wrote unarr as an
alternative to libarchive which didn't have support for parsing filters or
solid compression at the time.

While (lib)unarr was started with the intent of providing unarr with a proper
cmake based build system suitable for packaging and cross-platform development,
it's focus has now been extended to provide code maintenance and to continue the
development of unarr, which no longer is maintained.

## Getting started

### Prebuilt packages
[![Packaging status](https://repology.org/badge/vertical-allrepos/unarr.svg)](https://repology.org/metapackage/unarr)

#### From OBS
[.deb package](https://software.opensuse.org//download.html?project=home%3Aselmf&package=libunarr)
[.rpm package](https://software.opensuse.org//download.html?project=home%3Aselmf%3Ayacreader-rpm&package=libunarr)

### Building from source

#### Dependencies

(lib)unarr can take advantage of the following libraries if they are present:

* bzip2
* xz / libLZMA
* zlib

#### CMake

```bash
mkdir build
cd build
cmake ..
make
```

... as a static library

```bash
cmake .. -DBUILD_SHARED_LIBS=OFF
```

By default, (lib)unarr will try to detect and use system libraries like bzip2,
xz/LibLZMA and zlib. If this is undesirable, you can override this behavior by
specifying:

```bash
cmake .. -DUSE_SYSTEM_BZ2=OFF -DUSE_SYSTEM_LZMA=OFF -DUSE_SYSTEM_ZLIB=OFF
```

Install

```bash
make install
```

#### Testing

Unarr supports unit tests, integration tests and fuzzing.


```bash
cmake .. -DBUILD_UNIT_TESTS=ON -DBUILD_INTEGRATION_TESTS=ON
```

To build the unit tests, the *cmocka* unit testing framework is required.

Building the integration tests also enables the *unarr-test* executable
which can be used to run additional tests on user-provided archive files.

Building the fuzzer target will provide a coverage-guided fuzzer based
on llvm libfuzzer. It should be treated as a stand-alone target.

```bash
cmake .. -DBUILD_FUZZER=ON
```

All tests can be run using ctest or their respective executables.

## Usage

### Examples

Check [unarr.h](unarr.h) and [unarr-test](test/main.c) to get a general feel
for the api and usage.

The unarr-test sample application can be used to test archives.

To build it, use:

```bash
cmake .. -DBUILD_INTEGRATION_TESTS=ON
```

## Limitations

Unarr was written for comic book archives, so it currently doesn't support:

* password protected archives
* self extracting archives
* split archives

### 7z support

7z support for large files with solid compression is currently limited by a
known performance problem in the ANSI-C based LZMA SDK
(see https://github.com/zeniko/unarr/issues/4).

Fixing this problem will require modification or replacement of the LZMA SDK
code used.

### Rar support

RAR5 is currently not supported. There are plans to add this in a future version,
but as of now this is still work in progress.

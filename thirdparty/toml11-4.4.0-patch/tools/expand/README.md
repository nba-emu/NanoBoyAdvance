# expand

`expand` is a tool designed to process `#include "..."` directives within a file, generating a single `single_include/toml.hpp` file.

`expand` is optimized specifically for the structure of `toml11`.
It assumes that the argument provided is `include/toml.hpp` and that there are files separated into `fwd/` and `impl/` directories.
Therefore, it is not a general-purpose tool.

## Usage

```console
$ g++-13 -std=c++20 -O2 main.cpp -o expand
$ ./expand ../../include/toml.hpp > ../../single_include/toml.hpp
```

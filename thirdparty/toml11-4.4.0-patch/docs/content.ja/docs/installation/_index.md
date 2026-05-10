+++
title = "installation"
type  = "docs"
weight = 1
+++

# installation

## `single_include`を使用する

`single_include/toml.hpp`は、`toml11`が持つ全ての機能を単一のファイルにまとめたシングルファイル・ヘッダオンリーライブラリです。

これを`INCLUDE_PATH`が通っている箇所にコピーして`#include <toml.hpp>`とするのが最も単純な使用方法です。

MITライセンスの許諾表示はコメントと`toml:license_notice()`関数の両方に含まれます。
ソースコードを公開せずに再頒布する場合は、toml11のライセンスファイルをコピーして同梱するか、この関数を呼び出せるようにしておいてください。

## toml11をクローンし、`cmake`を使って使用する

`toml11`を`git submodule`などによって自身のレポジトリ下に配置した場合、`cmake`を使用している場合は`add_subdirectory(toml11)`のようにすることで使用可能になります。

```cmake
add_subdirectory(toml11)
add_executable(main main.cpp)
target_link_libraries(main PUBLIC toml11::toml11)
```

`toml11`は自身がルートプロジェクトのときのみ、テストとインストールを行います。

### CMake `FetchContent`

CMakeの `FetchContent`を使用することで、`build`ディレクトリに自動でダウンロードすることができます。

```cmake
include(FetchContent)
FetchContent_Declare(
  toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
  GIT_TAG        v4.4.0
)
FetchContent_MakeAvailable(toml11)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE toml11::toml11)
```

### CMake Package Manager (CPM)

[CMake package manager](https://github.com/cpm-cmake/CPM.cmake)を導入すると、以下のようにして使用することができます。

```cmake
include(cmake/CPM.cmake)

CPMAddPackage("gh:ToruNiina/toml11@4.4.0")

# OR

CPMAddPackage(
    NAME toml11
    GITHUB_REPOSITORY "ToruNiina/toml11"
    VERSION 4.4.0
    OPTIONS
    "TOML11_PRECOMPILE ON" # to pre-compile
    "TOML11_ENABLE_ACCESS_CHECK ON" # to use value.accessed()
    )

add_executable(main main.cpp)
target_link_libraries(main PUBLIC toml11::toml11)
```

## `cmake`を使用してインストールする

`toml11`をクローンしたのち、`cmake`を使ってインストールすることができます。

```console
$ cmake -B ./build/ -DTOML11_BUILD_TESTS=ON
$ cmake --install ./build/ --prefix=/opt/toml11
```

インストールの前にテストプログラムを実行する際は、最初に`-DTOML11_BUILD_TESTS=ON`を設定してください。

インストールが完了すれば、以下のようにして使用できます。

```cmake
find_package(toml11)
add_executable(main main.cpp)
target_link_libraries(main PRIVATE toml11::toml11)
```

## `cmake`を使用してコンパイルし、静的ライブラリを作成する

`cmake`の実行時に`-DTOML11_PRECOMPILE=ON`を定義することで、`toml11`の関数のうちコンパイルできるものを先にコンパイルして、全体のコンパイル時間を短縮することができます。

```console
$ cmake -B ./build/ -DTOML11_PRECOMPILE=ON
```

ライブラリをリンクする場合は、CMakeで

```cmake
target_link_libraries(your_target PUBLIC toml11::toml11)
```

とするか、ヘッダ内の関数の`inline`化を避けるためにコンパイラに`-DTOML11_COMPILE_SOURCES`を渡してください。

ただし、toml11は複数のC++バージョンに対応するため、`__cplusplus`の値などによって型を切り替えることがあります。
そのため、ビルドした際のバージョンと使用時のバージョンが異なる場合、リンクに失敗する可能性があります。
問題が生じた場合は`CMAKE_CXX_STANDARD`によって必要なバージョンを設定してコンパイルしてください。
それが難しい場合は、通常通りヘッダオンリーライブラリとして使用してください。

`find_package(toml11)`によって`TOML11_INCLUDE_DIR`が定義されます。
コンパイル済みライブラリとしてインストールした場合でも、 `TOML11_INCLUDE_DIR` を
`include_directories` に追加した上で `target_link_libraries` を
使用しないようにすれば、ヘッダオンリーライブラリとして使用可能です。

```cmake
find_package(toml11)
add_executable(main main.cpp)
# インクルードのみ可能にし、リンクを行わない
target_include_directories(main PRIVATE ${TOML11_INCLUDE_DIR})
```

## examplesをコンパイルする

`-DTOML11_BUILD_EXAMPLES=ON`とすることで、`examples/`をコンパイルできます。

```console
$ cmake -B ./build/ -DTOML11_BUILD_EXAMPLES=ON
$ cmake --build ./build/
```

`examples`の実行バイナリは`examples/`に生成されます。

## テストを実行する

テストをビルドするためには、`-DTOML11_BUILD_TESTS=ON`とします。

```console
$ git submodule update --init --recursive
$ cmake -B ./build/ -DTOML11_BUILD_TESTS=ON
$ cmake --build ./build/
$ ctest --test_dir ./build/
```

toml-lang/toml-testsを実行するには、`-DTOML11_BUILD_TOML_TESTS=ON`とします。
すると、`tests/`に`toml11_decoder`と`toml11_encoder`がビルドされます。

```console
$ git submodule update --init --recursive
$ cmake -B ./build/ -DTOML11_BUILD_TOML_TESTS=ON
$ cmake --build ./build/
$ ctest --test_dir ./build/
```

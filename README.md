# OITools::Stress-Tester

A little program for stress-testing in competitive programming.

This project is part of OITools Project.

This project is using [the Boost Library](https://www.boost.org/).

Only supports Windows now.

## How to use

run `oit-stress help`.

Customized checkers are recommended to be written with [testlib](https://github.com/MikeMirzayanov/testlib).

## Build & Install

Requirements:

* GCC (MinGW in Windows)
* CMake
* Boost Library

You may need administrator privileges during the following steps.

### Linux

Run the following commands under the source directory:

```bash
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build .
cmake --install . --prefix=${install_directory}
```

where `${install_directory}` is the directory to install in. If `--prefix` is not specified, it'll be `/usr/local/`.

### Windows

Run the following commands under the source directory:

```bash
cmake . -DCMAKE_BUILD_TYPE=Release -G"MinGW Makefiles"
cmake --build .
cmake --install . --prefix=${install_directory}
```

where `${install_directory}` is the directory to install in. If `--prefix` is not specified, it'll be `C:/Program Files/OITools/`.

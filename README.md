# OITools::Stress-Tester

A little program for stress-testing in competitive programming.

This project is part of OITools Project.

This project is using [the Boost Library](https://www.boost.org/).

~~Only supports Windows now~~ Only supports Windows and Linux now.

## How to use

Run `oit-stress --help` and you'll see.

## Build & Install

Requirements:

* GCC (MinGW or MSYS2 on Windows)
* CMake
* Boost Library

You may need administrator privileges during the following steps.

### Build on Linux

Run the following commands under the source directory:

```bash
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build .
cmake --install . --prefix=${install_directory} --strip
```

where `${install_directory}` is the directory to install in. If `--prefix` is not specified, it'll be `/usr/local/`.

### Build on Windows

It's not recommended to build on windows yourself unless you're expert at configuring environment. 

After setting up Boost and CMake properly, run the following commands under the source directory:

```bash
cmake . -DCMAKE_BUILD_TYPE=Release -G"MinGW Makefiles"
cmake --build .
cmake --install . --prefix=${install_directory} --strip
```

where `${install_directory}` is the directory to install in. If `--prefix` is not specified, it'll be `C:/Program Files/OITools/`.

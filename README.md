# Lox - C Edition

This repo contains an implementation of the bytecode VM for the [Lox](https://craftinginterpreters.com/)
programming language in C.

In the 'original' directory is a slightly modified version of my first time
implementing it.

I am currently in the middle of refactoring the project to be a bit more easily
extensible and maintainable. Some further optimizations are being made as well.

One of the main goals early on is to bring down a lot of the internal functionality
presented in [Wren](https://wren.io/), another language also written by
[Bob Nystrom](https://journal.stuffwithstuff.com/) which shares a lot of code with Lox.
Much of it is done either the same, or a little bit differently, but generally better.

A small example is the ParseRule table, which now uses macros for a much easier to
view, read, understand and extend table.

## Building

At the moment, since this is not a production project (yet), the only way to try
it is to build it yourself.

Currently, only Windows (via MSVC) and Linux are supported, however I plan on soon
adding the build toolchains for BSD and MacOS as well. That is a bit more difficult
for me as I do not own a system that runs either of those OSes.

All systems require the following to build:

- [CMake](https://cmake.org/)
- A C compiler. By default, it is set to check for MSVC on Windows. On Linux, it checks
first for clang, then for gcc. If neither are found, it defaults to 'cc' which is
generally installed by default in many distros.

The Linux build requires access to GNU readline.

If all above requirements are met, all systems can build via the following commands:

```bash
cmake -S . -B build
cmake --build build
```

The resulting binary can be found in ```build/Debug```, as debug builds are the default.

For release builds, enter the following:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

OR

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

This will compile the project with optimizations, and disable internal debugging
functionality.

The resulting binaries will be found in ```build/Release``` or
```build/RelWithDebInfo``` respectively.

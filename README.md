# Lox - C Edition

This repo contains my original implementation of the bytecode VM for the [Lox](https://craftinginterpreters.com/)
programming language in C.

The only real difference is the project surrounding it. I originally built it with Make
but moved to using CMake to make the build more cross-platform.

The lack of commit history is because this was moved to a fresh repo. I dirtied up
the original quite a lot when I decide to experiment with a bunch of things.

## Building

Windows, MacOS, and Linux are all supported.

All systems require the following to build:

- [CMake](https://cmake.org/)
- Basically any C compiler.
- GNU readline

Windows is the only exception for GNU readline as it is not available for Windows.

CMake will attempt to find and use specific compilers for specific platforms.
If not found, CMake should still automatically find a working C compiler if
one is installed.

For Windows users, I recommend getting [msys2](https://www.msys2.org/) and installing
`clang` to build with. MSVC should also work, but is very slow and is usually
not happy about compiling plain C.

### Clone the Project

```git clone https://github.com/Utecha/clox.git```

*You can use the ```--depth 1``` flag for a shallow clone*

### Setup the Build

```sh
cmake -S . \
    -B build \
    -DCMAKE_BUILD_TYPE=[Debug,Release] \
    -DNAN_TAGGING=[OFF,ON]
```

For the ```-DCMAKE_BUILD_TYPE``` and ```-DNAN_TAGGING``` options, choose one
of the two options you see in the brackets above.

The "NaN Tagging" feature is an optimization to how Values are stored in memory.
Without going into great detail, it provides a pretty significant improvement in
performance and memory usage.

Even in the early portions of the rewrite, it is available to be enabled. I did not
wait until the end as was done in the book/original.

### Build

Replace ```x``` with however many threads you want to use to compile. The more, the
better. Of course, don't go beyond how many you *actually* have (though it doesn't
really *hurt* to.)

```sh
cmake --build build -j x
```

The final build result can be found in ```build/type```, with ```type``` being
the build type you chose.

# Release Notes

## Lox 'Modern' Version 0.0.1

### Preamble

For some time now I've been slowly working on a rewrite of the project in
modern C23. I have run into some bugs in that rewrite that I have not yet
found the solution to, as well as having gone back and forth on ideas for
what I actually want to do with the rewrite.

All of that work exists in a separate copy of the repo that has the commits,
but none of the code was ever pushed. Starting from now, all of the code will
be *thoroughly* reviewed after it is written and will actually be pushed with
each commit once it has been confirmed stable.

Much like the original implementation (in the original, now deleted repo),
each version will correspond to the completion of a specific feature, typically
a specific chapter of the book.

Unlike the original implementation, there may be multiple smaller commits that
lead up to the completion of each version. Code will only be pushed once each
version (feature) is complete, and only after significant review to ensure
correctness and stability.

One final note: The ultimate goal in the end is to allow embedding of the new
"modern" version. The header `lox.h` will feature the public API.

Later on, once more of the language has been reimplemented, I will be switching
over to building the project as a library with a separate executable that
statically links with the new `liblox`. The API is being written with prefixes
ahead of time to prepare for this.

### Features

### Internal

- Added `lox.h`, replacing the former `common.h`.
- Renamed `VMResult` to `LoxRuntimeResult`.
- Added `lox_vm.h`.
- Added `lox_memory.h` and `lox_memory.c`.
- The sizing of memory allocations now uses the signed size type `ssize_t`
rather than unsigned `size_t` to prevent a number of bugs related to unsigned
sizes.
- Added `lox_array.h` for generically defining and working with custom dynamic
array types.

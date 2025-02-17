# Changelog

- [Refactor 001](#refactor-001)

## Refactor 001

I have been wanting to refactor this project for a while in order to make it a bit more
extensible. After some time thinking about it, I realized that with what I plan on doing
I might as well practically rewrite it. Therefore, I have done so.

The overwhelming majority of the changes I have/will make were inspired by the [Wren](https://wren.io/)
language.

So far, everything up to strings have been implemented (if following the book). Some notable changes:

- The compilers ParseRule table has been refactored to use macros, making it much cleaner and easier to
extend.
- I am still using manually implemented dynamic arrays for things like the bytecode, line numbers, values, etc.
However, I have moved all of those to their own module that will only exist for a time. I plan on moving to a
more generic system in the near future but I needed something in place just to get the rest of the VM up
and running now.
- With that in mind, the Chunk has been refactored and now uses those arrays. This also will fall by the wayside
at some point, with its functions being refactored where appropriate as I shift to compiling this data into
the ObjFn's directly.
- Multiple VM's can be instantiated as there is no singular, global VM anymore. This goes for
the lexer, parser, and compiler as well. All structs are passed by pointer into various functions.
- The ObjString (and later, others like the CLOSURE) have been updated to use flexible array members. A new
macro 'ALLOCATE_FLEX' was added for that purpose. The system of allocation also comes from Wren. There is now only
a single contiguous allocation for an ObjString rather than 2 separate ones (one for the Obj, one for the char array)
which in turn saves memory and provides a boost in speed to operations involving strings.
- The bare bones of the conditional operator (?:) are implemented, but non-functional. This will remain the case until
statements (and therefore jump-based instructions) are introduced.

# Changelog

- [Refactor 001](#refactor-001)
- [Refactor 002](#refactor-002)

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

## Refactor 002

We've about reached the point where the refactor needs a refactor! Only half joking...

In all seriousness, we're back up to the point where statements and declarations have been added. This includes...
- Variables, both local and global
- If statements
- Loops, complete with break and continue keywords
- Logical 'and' and 'or' reimplemented with their own special instructions. These are faster than the original
implementation.
- The conditional operator has also finally been implemented now that jump instructions exist.

One major difference now is that the parentheses surrounding the condition/clauses for the if statement and
loops are no longer required. In the case of our lovely C-style for loop, this wasn't exactly super straight
forward to do without breaking compatibility with the original Lox. As a result, the for loop still tries to
check for the parentheses and takes note of them. Take this example:

```c
for (var i = 0; i < 10; i = i + 1) {
    print i;
}
```

That should work exactly as expected. So should this:

```c
for var i = 0; i < 10; i = i + 1 {
    print i;
}
```

However, if it detects a left parenthesis with no matching right parenthesis:

```
[line 1] Error at '1': Left parenthesis found with no matching right
```

As you can imagine, you get a similar error if it finds a right parenthesis without a matching left.

This also opens another problem. Take this example:

```c
for var i = 0; i < 10; i = i + 1 print i;
```

This is really quite difficult to read and is not particularly ideal. As a result, it is an error to have a single-line or
single-statement for loop without one of these conditions being met:
1. The clauses are surrounded by parentheses as shown in the first example
2. The body is surrounded by braces (in a block scope) as shown in the second example.

Another potential solution could have been to match for a semicolon, however I feel the route I chose is better for readability while
trying to maintain compatability as much as possible. This does also essentially apply to for loops that span multiple lines, even
if they only contain one statement much like the first two examples.

I am considering down-patching from Wren the handling of new lines to a certain extent. In Wren, newlines are significant and are used
to denote the end of a statement or expression. I do not necessarily want it to function in that way for Lox, I *do* like my semicolons,
however it *CAN* be useful in certain scenarios where I may want a new line to be significant. A good example is for the for loop problem.
I could use that newline significance to allow something like this:

```c
for var i = 0; i < 10; i = i + 1
    print i;
```

That is nowhere near as difficult to read as the single-line variant and in my opinion, should be allowed. It is not possible to implement
without first generating new line tokens and implementing the functionality to occasionally consider new lines as significant. For now, this
remains on the back burner while I focus on getting the rest of the language reimplemented.

One thing I added *light* support for was the 'const' keyword. It exists as a keyword, however it has no significance in the compiler as of
yet.

I tried implementing it along with variables, however I ran into several problems when it came to determining if a global variable is constant
when compiling a variable assignment expression. The only real solution I could think of in the moment was to add a separate table and separate
plumbing for constants, however that still presents challenges when hooking into assignment expressions. Theoretically I could make that work
if, when variables are defined, checks are made to ensure that variables and constants cannot shadow each other within the same scope. Consider:

```c
const a = 16;
// ... later on
var a = 32;
```

Checking for such cases and making sure it is an error when it happens could ensure that I could safely assume a variable is a constant if it
appears in the global constants table and not the global variables table. There is yet still a problem, though... that is INCREDIBLY slow.
Think about it. That would require two separate hash table look-ups. Even with a super fast hash table implementation, that is one more
lookup than is required of variables, plus both constants and variables will have an extra check to prevent shadowing when defined. That's
enough extra overhead to not be worth it, in my opinion. For now, I'll have to ponder on a better way of implementing it in full.
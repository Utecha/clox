#ifndef lox_common_h
#define lox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct LoxVM LoxVM;
typedef struct Compiler Compiler;
typedef struct Table Table;

#ifdef DEBUG
    /*
    * If enabled, the VM will dump the generated tokens to
    * the standard error stream.
    */
    #define DUMP_TOKENS 0

    /*
    * If enabled, the VM will disassemble and dump an entire
    * compiled bytecode chunk to the standard error stream.
    */
    #define DUMP_CHUNK 1

    /*
    * If enabled, the execution of the compiler itself will
    * be traced.
    *
    * NOTE: Not yet implemented
    */
    #define TRACE_COMPILER 0

    /*
    * If enabled, the VM will disassemble each bytecode
    * instruction. Inbetween each instruction is the VMs
    * stack at that point in the bytecode.
    *
    * The result of all of that is dumped to the standard
    * error stream.
    */
    #define TRACE_INSTRUCTIONS 0

    /*
    * If enabled, outputs data about objects being allocated,
    * reallocated, or collected to the standard error stream.
    *
    * This includes data such as...
    *
    * - The object type
    * - The number of bytes allocated (or freed)
    * - The difference between a previous allocation and a
    *   reallocation
    * - The memory address of the object
    */
    #define LOG_GC 0

    /*
    * If enabled, the GC will be stress tested by significantly
    * easing the requirement before a collection is run.
    * This causes more total collections to be run which can
    * help catch bugs in the GC.
    *
    * This is really only useful when hacking on the VM.
    */
    #define STRESS_GC 0
#endif // DEBUG

/*
* If enabled, the core Value representation uses a
* NaN-tagged double. Otherwise, it uses a larger and more
* conventional structure.
*
* The former is far more compact and significantly faster,
* however the latter is better for debugging and is
* overall more portable.
*/
#define NAN_TAGGING 0

#endif // lox_common_h

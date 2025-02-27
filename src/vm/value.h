#ifndef lox_value_h
#define lox_value_h

#include "common.h"
#include "utils.h"

typedef enum
{
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
} ObjType;

typedef struct Obj
{
    ObjType type;
    bool isDark;
    struct Obj *next;
} Obj;

typedef struct ObjString ObjString;

#if NAN_TAGGING

typedef uint64_t Value;

#else

typedef enum
{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

typedef struct
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
        Obj *obj;
    } as;
} Value;

#define BOOL_VAL(value)     ((Value) { VAL_BOOL, { .boolean = value } })
#define NIL_VAL             ((Value) { VAL_NIL, { .number = 0 } })
#define NUMBER_VAL(value)   ((Value) { VAL_NUMBER, { .number = value } })
#define OBJ_VAL(object)     ((Value) { VAL_OBJ, { .obj = (Obj *)object } })

#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)
#define AS_OBJ(value)       ((value).as.obj)

#define IS_BOOL(value)      ((value).type == VAL_BOOL)
#define IS_NIL(value)       ((value).type == VAL_NIL)
#define IS_NUMBER(value)    ((value).type == VAL_NUMBER)
#define IS_OBJ(value)       ((value).type == VAL_OBJ)

#endif // NAN_TAGGING

DECLARE_ARRAY(Value, Value);

typedef struct
{
    ByteArray code;
    LineArray lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int addConstant(Chunk *chunk, Value value);
int getLine(Chunk *chunk, int offset);

typedef struct
{
    Obj obj;
    int arity;
    Chunk chunk;
    ObjString *name;
    // ByteArray code;
    // ValueArray constants;
    // struct
    // {
    //     ObjString *name;
    //     LineArray lines;
    // } debug;
} ObjFn;

typedef struct
{
    ObjFn *function;
    uint8_t *ip;
    Value *slots;
} CallFrame;

typedef Value (*NativeFn)(int argc, Value *args);

typedef struct
{
    Obj obj;
    NativeFn function;
    int arity;
} ObjNative;

struct ObjString
{
    Obj obj;
    uint32_t length;
    uint32_t hash;
    char value[];
};

#define OBJ_TYPE(value)     (AS_OBJ(value)->type)

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && OBJ_TYPE(value) == type;
}

#define AS_FUNCTION(value)  ((ObjFn *)AS_OBJ(value))
#define AS_NATIVE(value)    ((ObjNative *)AS_OBJ(value))
#define AS_NATIVE_FN(value) (AS_NATIVE(value)->function)
#define AS_STRING(value)    ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value)   (AS_STRING(value)->value)

#define IS_FUNCTION(value)  isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value)    isObjType(value, OBJ_NATIVE)
#define IS_STRING(value)    isObjType(value, OBJ_STRING)

#define CONST_STRING(vm, text) newStringLength((vm), (text), sizeof(text) - 1)

ObjFn *newFunction(LoxVM *vm);
ObjNative *newNative(LoxVM *vm, NativeFn function, int arity);
ObjString *newStringLength(LoxVM *vm, const char *chars, size_t length);
ObjString *newString(LoxVM *vm, char *chars);

void printValue(Value value);
bool isFalsey(Value value);
bool valuesEqual(Value a, Value b);

#endif // lox_value_h

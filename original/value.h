#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include <string.h>

typedef struct Obj Obj;
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

#define SIGN_BIT    ((uint64_t)0x8000000000000000)
#define QNAN        ((uint64_t)0x7ffc000000000000)

#define TAG_NIL     1 // 01
#define TAG_FALSE   2 // 10
#define TAG_TRUE    3 // 11

typedef uint64_t Value;

#define IS_BOOL(value)      value_is_bool(value)
#define IS_NIL(value)       ((value) == NIL_VAL)
#define IS_NUMBER(value)    (((value) & QNAN) != QNAN)
#define IS_OBJ(value)       (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(value)      ((value) == TRUE_VAL)
#define AS_NUMBER(value)    value_to_num(value)
#define AS_OBJ(value)       ((Obj *)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

#define BOOL_VAL(boolean)   ((boolean) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL           ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL            ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL             ((Value)(uint64_t)(QNAN | TAG_NIL))
#define NUMBER_VAL(num)     num_to_value(num)
#define OBJ_VAL(obj)        (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

static inline double value_to_num(Value value)
{
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}

static inline Value num_to_value(double num)
{
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

#else // NAN_BOXING

typedef enum {
    VL_BOOL,
    VL_NIL,
    VL_NUMBER,
    VL_OBJ
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj *obj;
    } as;
} Value;

// Is this value one of the Lox Value types?
#define IS_BOOL(value)      ((value).type == VL_BOOL)
#define IS_NIL(value)       ((value).type == VL_NIL)
#define IS_NUMBER(value)    ((value).type == VL_NUMBER)
#define IS_OBJ(value)       ((value).type == VL_OBJ)

// The underlying C value converted back from a Lox Value
#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)
#define AS_OBJ(value)       ((value).as.obj)

// A Lox Value converted from a C value
#define BOOL_VAL(value)     ((Value){ VL_BOOL, { .boolean = value } })
#define NIL_VAL             ((Value){ VL_NIL, { .number = 0 } })
#define NUMBER_VAL(value)   ((Value){ VL_NUMBER, { .number = value } })
#define OBJ_VAL(object)     ((Value){ VL_OBJ, { .obj = (Obj *)object } })

#endif // NAN_BOXING

static inline bool value_is_bool(Value value)
{
#ifdef NAN_BOXING
    return value == TRUE_VAL || value == FALSE_VAL;
#else
    return IS_BOOL(value);
#endif
}

typedef struct {
    int capacity;
    int count;
    Value *values;
} ValueArray;

bool values_equal(Value a, Value b);
void init_value_array(ValueArray *array);
void free_value_array(ValueArray *array);
void write_value_array(ValueArray *array, Value value);
void print_value(Value value);

#endif // CLOX_VALUE_H

#ifndef lox_value_h
#define lox_value_h

#include "common.h"

typedef enum
{
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

#define AS_STRING(value)    ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value)   (((ObjString *)AS_OBJ(value))->value)

#define IS_STRING(value)    isObjType(value, OBJ_STRING)

void hashString(ObjString *string);
ObjString *newStringLength(LoxVM *vm, const char *chars, size_t length);
ObjString *newString(LoxVM *vm, char *chars);

void printValue(Value value);
bool isFalsey(Value value);
bool valuesEqual(Value a, Value b);

#endif // lox_value_h

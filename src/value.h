#ifndef lox_value_h
#define lox_value_h

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum
{
    OBJ_STRING,
} ObjType;

typedef enum
{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ,
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

struct Obj
{
    ObjType type;
    Obj *next;
};

struct ObjString
{
    Obj obj;
    int length;
    uint32_t hash;
    char *data;
};

typedef struct
{
    int count;
    int capacity;
    Value *values;
} ValueArray;

#define BOOL_VAL(value)     ((Value){ VAL_BOOL, { .boolean = value } })
#define NIL_VAL             ((Value){ VAL_NIL, { .number = 0 } })
#define NUMBER_VAL(value)   ((Value){ VAL_NUMBER, { .number = value } })
#define OBJ_VAL(object)     ((Value){ VAL_OBJ, { .obj = (Obj *)object } })

#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)
#define AS_OBJ(value)       ((value).as.obj)

#define IS_BOOL(value)      ((value).type == VAL_BOOL)
#define IS_NIL(value)       ((value).type == VAL_NIL)
#define IS_NUMBER(value)    ((value).type == VAL_NUMBER)
#define IS_OBJ(value)       ((value).type == VAL_OBJ)

#define OBJ_TYPE(value)     (AS_OBJ(value)->type)

#define AS_STRING(value)    ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value)   (((ObjString *)AS_OBJ(value))->data)

static inline bool is_obj_type(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#define IS_STRING(value)    is_obj_type(value, OBJ_STRING)

// Value Functions
void print_value(Value value);
bool values_equal(Value a, Value b);

// Object Functions
ObjString *copy_string(LoxVM *vm, const char *data, int length);
ObjString *take_string(LoxVM *vm, char *data, int length);
bool string_equals_cstring(ObjString *string, const char *cstring, int length, uint32_t hash);

// ValueArray Functions
void init_value_array(ValueArray *array);
void free_value_array(LoxVM *vm, ValueArray *array);
void write_value_array(LoxVM *vm, ValueArray *array, Value value);

#endif // lox_value_h

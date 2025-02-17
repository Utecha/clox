#ifndef lox_arrays_h
#define lox_arrays_h

#include "value.h"

typedef struct
{
    int capacity;
    int count;
    uint8_t *data;
} ByteArray;

typedef struct
{
    int number;
    int offset;
} Line;

typedef struct
{
    int capacity;
    int count;
    Line *data;
} LineArray;

typedef struct
{
    int capacity;
    int count;
    Value *data;
} ValueArray;

void initByteArray(ByteArray *array);
void freeByteArray(ByteArray *array);
void writeByteArray(ByteArray *array, uint8_t byte);

void initLineArray(LineArray *array);
void freeLineArray(LineArray *array);
void writeLineArray(LineArray *array, Line line);

void initValueArray(ValueArray *array);
void freeValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);

#endif // lox_arrays_h

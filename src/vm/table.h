#ifndef lox_table_h
#define lox_table_h

#include "value.h"

typedef struct
{
    ObjString *key;
    Value value;
} Entry;

struct Table
{
    uint32_t capacity;
    uint32_t count;
    Entry *entries;
};

void tableInit(Table *table);
void tableFree(Table *table);
bool tableDelete(Table *table, ObjString *key);
bool tableGet(Table *table, ObjString *key, Value *value);
bool tableSet(Table *table, ObjString *key, Value value);
void tableAddAll(Table *from, Table *to);
ObjString *tableFindString(Table *table, const char *chars, uint32_t length, uint32_t hash);

#endif // lox_table_h

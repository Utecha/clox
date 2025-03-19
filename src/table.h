#ifndef lox_table_h
#define lox_table_h

#include "value.h"

typedef struct
{
    ObjString *key;
    Value value;
} Entry;

typedef struct
{
    int count;
    int capacity;
    Entry *entries;
} Table;

void init_table(Table *table);
void free_table(LoxVM *vm, Table *table);
bool get_from_table(Table *table, ObjString *key, Value *value);
bool set_to_table(LoxVM *vm, Table *table, ObjString *key, Value value);
bool remove_from_table(Table *table, ObjString *key);
void add_all_to_table(LoxVM *vm, Table *from, Table *to);
ObjString *find_interned_string(Table *table, const char *data, int length, uint32_t hash);

#endif // lox_table_h

#include "memory.h"
#include "table.h"
#include "vm.h"

#define TABLE_MAX_LOAD_FACTOR 0.75

void init_table(Table *table)
{
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void free_table(LoxVM *vm, Table *table)
{
    DEALLOCATE_ARRAY(vm, Entry, table->entries, table->capacity);
    init_table(table);
}

static Entry *find_entry(Entry *entries, int capacity, ObjString *key)
{
    uint32_t index = key->hash & (capacity - 1);
    Entry *tombstone = NULL;

    for (;;)
    {
        Entry *entry = &entries[index];
        if (entry->key == NULL)
        {
            if (IS_NIL(entry->value))
            {
                // Empty entry
                return tombstone != NULL ? tombstone : entry;
            }
            else
            {
                // We found a tombstone
                if (tombstone == NULL) tombstone = entry;
            }
        }
        else if (entry->key == key)
        {
            // We found the key
            return entry;
        }

        index = (index + 1) & (capacity - 1);
    }
}

bool get_from_table(Table *table, ObjString *key, Value *value)
{
    if (table->count == 0) return false;

    Entry *entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

static void resize_table(LoxVM *vm, Table *table, int capacity)
{
    Entry *entries = ALLOCATE_ARRAY(vm, Entry, capacity);
    for (int i = 0; i < capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (entry->key == NULL) continue;

        Entry *dest = find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    DEALLOCATE_ARRAY(vm, Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool set_to_table(LoxVM *vm, Table *table, ObjString *key, Value value)
{
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD_FACTOR)
    {
        int capacity = GROW_CAPACITY(table->capacity);
        resize_table(vm, table, capacity);
    }

    Entry *entry = find_entry(table->entries, table->capacity, key);
    bool is_new_key = entry->key == NULL;
    if (is_new_key && IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return is_new_key;
}

bool remove_from_table(Table *table, ObjString *key)
{
    if (table->count == 0) return false;

    Entry *entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // Replace the entry with a 'tombstone'
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

void add_all_to_table(LoxVM *vm, Table *from, Table *to)
{
    for (int i = 0; i < from->capacity; i++)
    {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL)
        {
            set_to_table(vm, to, entry->key, entry->value);
        }
    }
}

ObjString *find_interned_string(Table *table, const char *data, int length, uint32_t hash)
{
    if (table->count == 0) return NULL;

    uint32_t index = hash & (table->capacity - 1);
    for (;;)
    {
        Entry *entry = &table->entries[index];
        if (entry->key == NULL)
        {
            // Stop if we find an empty non-tombstone entry
            if (IS_NIL(entry->value)) return NULL;
        }
        else if (string_equals_cstring(entry->key, data, length, hash))
        {
            // We found it
            return entry->key;
        }

        index = (index + 1) & (table->capacity - 1);
    }
}

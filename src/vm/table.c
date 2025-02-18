#include <string.h>

#include "memory.h"
#include "table.h"

#define MAX_LOAD_PERCENTAGE 75

void tableInit(Table *table)
{
    table->capacity = 0;
    table->count = 0;
    table->entries = NULL;
}

void tableFree(Table *table)
{
    FREE(Entry, table->entries, table->capacity);
    tableInit(table);
}

static Entry *tableFindEntry(Entry *entries, uint32_t capacity, ObjString *key)
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

static void tableResize(Table *table, uint32_t capacity)
{
    Entry *entries = ALLOCATE_ARRAY(Entry, capacity);
    for (uint32_t i = 0; i < capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    for (uint32_t i = 0; i < table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (entry->key == NULL) continue;

        Entry *dest = tableFindEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableDelete(Table *table, ObjString *key)
{
    if (table->count == 0) return false;

    // Find the entry
    Entry *entry = tableFindEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // Place a tombstone in the entry
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

bool tableGet(Table *table, ObjString *key, Value *value)
{
    if (table->count == 0) return false;

    Entry *entry = tableFindEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

bool tableSet(Table *table, ObjString *key, Value value)
{
    if (table->count + 1 > table->capacity * (MAX_LOAD_PERCENTAGE / 100))
    {
        uint32_t capacity = GROW_CAPACITY(table->capacity);
        tableResize(table, capacity);
    }

    Entry *entry = tableFindEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    if (isNewKey && IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

void tableAddAll(Table *from, Table *to)
{
    for (uint32_t i = 0; i < from->capacity; i++)
    {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL) tableSet(to, entry->key, entry->value);
    }
}

ObjString *tableFindString(Table *table, const char *chars, uint32_t length, uint32_t hash)
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
        else
        {
            bool equalLength = entry->key->length == length;
            bool equalHash = entry->key->hash = hash;
            bool equalMem = memcmp(entry->key->value, chars, length) == 0;

            if (equalLength && equalHash && equalMem) return entry->key;
        }

        index = (index + 1) & (table->capacity - 1);
    }
}

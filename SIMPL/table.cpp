#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"

template <typename key, typename value>
Table<key, value>::Table(){}

template <typename key, typename value>
bool Table<key, value>::tableSet(key *key, value value)
{
    table[key] = value;
    return true;
}

// returns false if table empty/key not in table, otherwise caller gets the value stored in value ptr
template <typename key, typename value>
bool Table<key, value>::tableGet(key *key, value *value)
{
    if (table.find(key) == table.end())
    {
        return false;
    }

    *value = table.at(key);
    return true;
}

template <typename key, typename value>
bool Table<key, value>::tableDelete(key *key)
{
    if (table.find(key) == table.end())
        return false;

    table.erase(key);
    return true;
}

template <typename key, typename value>
void Table<key, value>::tableAddAll(Table *from)
{
    // 
}

template <typename key, typename value>
ObjString *Table<key, value>::tableFindString(ObjString* str)
{
    if (table.find(str) != table.end()) 
    {
        return str;
    }
    else
    {
        return NULL;
    }
}

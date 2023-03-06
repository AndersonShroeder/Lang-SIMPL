#include <stdlib.h>
#include <string.h>
#include "table.hh"


template <typename key, typename value, typename hashFunction>
bool Table<key, value, hashFunction>::tableSet(key _key, value _value)
{
    bool exists = false;

    if (!tableFind(_key))
    {
        exists = true;
    }

    table[_key] = _value;
    return exists;
}

// returns false if table empty/key not in table, otherwise caller gets the value stored in value ptr
template <typename key, typename value, typename hashFunction>
bool Table<key, value, hashFunction>::tableGet(key _key, value _value)
{
    if (table.find(_key) == table.end())
    {
        return false;
    }

    _value = table.at(_key);
    return true;
}

template <typename key, typename value, typename hashFunction>
bool Table<key, value, hashFunction>::tableDelete(key _key)
{
    if (table.find(_key) == table.end())
        return false;

    table.erase(_key);
    return true;
}
  
template <typename key, typename value, typename hashFunction>
void Table<key, value, hashFunction>::tableAddAll(Table from)
{
    // 
}

template <typename key, typename value, typename hashFunction>
key Table<key, value, hashFunction>::tableFind(key _search)
{
    return table.find(_search) != table.end() ? _search : nullptr;
}

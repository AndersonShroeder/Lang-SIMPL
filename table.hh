#ifndef simpl_table_h
#define simpl_table_h

#include "values.hh"

template <typename key, typename value, typename hashFunction, typename equalityFunction>
class Table
{
public:
    std::unordered_map<key, value, hashFunction, equalityFunction> table;

    Table()
    {
        table = std::unordered_map<key, value, hashFunction, equalityFunction>();
    }

    bool tableSet(key _key, value _value);

    bool tableGet(key _key, value& _value);

    bool tableDelete(key _key);

    void tableAddAll(Table from);

    key tableFind(key _search);
};

#endif
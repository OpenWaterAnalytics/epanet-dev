/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "hashtable.h"
using namespace std;

//-----------------------------------------------------------------------------

// Insertion function

void HashTable::insert(string* key, Element* value)
{
    table[*key] = value;
}

//-----------------------------------------------------------------------------

// Count function

size_t HashTable::getCount()
{
    return table.size();
}

//-----------------------------------------------------------------------------

// Retrieval function

Element* HashTable::find(const string& key)
{
    auto it = table.find(key);
    if (it != table.end()) return it->second;
    return nullptr;
}

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

 // TO DO:
 // - allow user to set initial size of the hash table
 // - implement a dynamic hash table that grows as needed

#include "hashtable.h"
using namespace std;

const unsigned int HTMAXSIZE = 2048;  //1999;   // Max. size of hash table


struct HashTableItem                   // Hash table data structure
{
    std::string*   key;
    Element*       value;
    HashTableItem* next;
};

//-----------------------------------------------------------------------------

// Constructor -- create a hash table with HTMAXSIZE entries

HashTable::HashTable() : ht(new HTitem[HTMAXSIZE])
{
    for (size_t i = 0; i < HTMAXSIZE; i++) ht[i] = 0;
    count = 0;
}

//-----------------------------------------------------------------------------

// Destructor

HashTable::~HashTable()
{
    HTitem item;
    HTitem nextItem;

    // delete all items in the hash table
    for (size_t i = 0; i < HTMAXSIZE; i++)
    {
        item = ht[i];
        while (item)
        {
            nextItem = item->next;
            delete item;
            item = nextItem;
        }
    }

    // delete the table
    delete [] ht;
    count = 0;
}

//-----------------------------------------------------------------------------

// Insertion function

int HashTable::insert(string* key, Element* value)
{
    unsigned int i = hash(*key);
    if ( i >= HTMAXSIZE ) return 0;
    HashTableItem* item = new HashTableItem;
    if (!item) return 0;
    item->key = key;
    item->value = value;
    item->next = ht[i];
    ht[i] = item;
    count++;
    return 1;
}

//-----------------------------------------------------------------------------

// Count function

size_t HashTable::getCount()
{
    return count;
}

//-----------------------------------------------------------------------------

// Retrieval function

Element* HashTable::find(const string& key)
{
    unsigned int i = hash(key);
    if ( i >= HTMAXSIZE ) return nullptr;
    HTitem item = ht[i];
    while (item)
    {
        if ( *(item->key) == key ) return item->value;
        item = item->next;
    }
    return nullptr;
}

//-----------------------------------------------------------------------------

// Hashing function
// (Bernstein's function where (hash << 5) + hash  = 33 * hash)

unsigned int HashTable::hash(const string& str)
{
    unsigned long hash = 5381;
    for (unsigned int i = 0; i < str.size(); i++)
    {
        hash = ((hash << 5) + hash) + str[i]; /* hash * 33 + c */
    }
    return hash % HTMAXSIZE;
}

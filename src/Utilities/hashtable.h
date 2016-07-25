/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file hashtable.h
//! \brief Describes the HashTable class.

#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <string>
#include <map>

class Element;

//! \class HashTable
//! \brief A hash table for element ID names and their objects.
//! \note The table is case sensitive.

class HashTable
{
  public:

    void     insert(std::string* key, Element* value);
    Element* find(const std::string& key);
    size_t   getCount();

  private:

    std::map<std::string, Element*> table;
};

#endif

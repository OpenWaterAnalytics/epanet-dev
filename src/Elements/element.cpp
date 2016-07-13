/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "element.h"
//#include "mempool.h"
using namespace std;

Element::Element(string name_) :
    name(name_),
    index(-1)
{}

Element::~Element()
{}

/*
void *operator new( size_t num_bytes, MemPool *pool)
{
    return pool->alloc( num_bytes);
}
*/

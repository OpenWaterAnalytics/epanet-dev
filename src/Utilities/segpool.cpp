/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "segpool.h"
#include "Utilities/mempool.h"

#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------

SegPool::SegPool()
{
    memPool = new MemPool();
    freeSeg = nullptr;
    segCount = 0;
}

//-----------------------------------------------------------------------------

SegPool::~SegPool()
{
//    cout << "\n segCount = " << segCount << "\n";

    delete memPool;
}

//-----------------------------------------------------------------------------

void SegPool::init()
{
    segCount = 0;
    memPool->reset();
    freeSeg = nullptr;
}

//-----------------------------------------------------------------------------

Segment* SegPool::getSegment(double v, double c)
{
    // ... if there's a free segment available then use it
    Segment* seg;
    if ( freeSeg )
    {
       seg = freeSeg;
       freeSeg = seg->next;
    }

    // ... otherwise create a new one from the memory pool
    else
    {
        seg = (Segment *) memPool->alloc(sizeof(Segment));
        segCount++;
    }

    // ... assign segment's volume and quality
    if ( seg )
    {
        seg->v = v;
        seg->c = c;
        seg->next = nullptr;
    }
    return seg;
}

//-----------------------------------------------------------------------------

void SegPool::freeSegment(Segment* seg)
{
    seg->next = freeSeg;
    freeSeg = seg;
}

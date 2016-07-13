/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

/*
**  This code is based on "A simple fast memory allocation package"
**  by Steve Hill in Graphics Gems III, David Kirk (ed.),
**  Academic Press, Boston, MA, 1992
*/

#include "mempool.h"

/*
**  ALLOC_BLOCK_SIZE - adjust this size to suit your installation - it
**  should be reasonably large otherwise you will be mallocing a lot.
*/

#define ALLOC_BLOCK_SIZE   64000       /*(62*1024)*/

struct MemBlock
{
    struct MemBlock *next;   /* Next Block          */
    char            *block,  /* Start of block      */
                    *free,   /* Next free in block  */
                    *end;    /* block + block size  */
};

static struct MemBlock* createMemBlock()
{
    struct MemBlock* memBlock = new struct MemBlock;
    if (memBlock)
    {
        memBlock->block = new char[ALLOC_BLOCK_SIZE];
        if (memBlock->block == nullptr)
        {
            delete memBlock;
            return nullptr;
        }
        memBlock->free = memBlock->block;
        memBlock->next = nullptr;
        memBlock->end = memBlock->block + ALLOC_BLOCK_SIZE;
    }
    return memBlock;
}

static void deleteMemBlock(struct MemBlock* memBlock)
{
    delete [] memBlock->block;
    delete memBlock;
}


// MemPool Constructor

MemPool::MemPool()
{
    first = createMemBlock();
    current = first;
}


// MemPool Destructor

MemPool::~MemPool()
{
    while (first)
    {
        current = first->next;
        deleteMemBlock(first);
        first = current;
    }
}

/*
**  alloc()
**
**  Use as a direct replacement for malloc().  Allocates
**  memory from the current pool.
*/
char* MemPool::alloc(std::size_t size)
{
    /*
    **  Align to 4 byte boundary - should be ok for most machines.
    **  Change this if your machine has weird alignment requirements.
    */
    size = (size + 3) & 0xfffffffc;

    if (!current) return nullptr;
    char* ptr = current->free;
    current->free += size;

    /* Check if the current block is exhausted. */

    if (current->free >= current->end)
    {
        /* Is the next block already allocated? */

        if (current->next)
        {
            /* re-use block */
            current->next->free = current->next->block;
            current = current->next;
        }
        else
        {
            /* extend the pool with a new block */
            current->next = createMemBlock();
            if (!current->next) return nullptr;
            current = current->next;
        }

        /* set ptr to the first location in the next block */

        ptr = current->free;
        current->free += size;
    }

    /* Return pointer to allocated memory. */

    return ptr;
}

/*
**  reset()
**
**  Reset the current pool for re-use.  No memory is freed,
**  so this is very fast.
*/

void  MemPool::reset()
{
    current = first;
    current->free = current->block;
}

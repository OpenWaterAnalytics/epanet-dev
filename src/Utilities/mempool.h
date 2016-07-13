/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file  mempool.h
//! \brief Describes the MemPool class.

#ifndef MEMPOOL_H_
#define MEMPOOL_H_

#include <cstddef>

struct MemBlock;

//! \class MemPool
//! \brief A simple pooled memory allocator.

class MemPool
{
  public:
    MemPool();
    ~MemPool();
    char* alloc(std::size_t size);
    void  reset();

  private:
    MemBlock* first;
    MemBlock* current;
};

#endif

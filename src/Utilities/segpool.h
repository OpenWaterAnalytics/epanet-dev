/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file segpool.h
//! \brief Describes the SegPool class used for water quality transport.

#ifndef SEGPOOL_H_
#define SEGPOOL_H_

class MemPool;

struct  Segment              //!< Volume segment
{
   double  v;                //!< volume (ft3)
   double  c;                //!< constituent concentration (mass/ft3)
   struct  Segment* next;    //!< next upstream volume segment
};

class SegPool
{
  public:
    SegPool();
    ~SegPool();
    void init();
    Segment* getSegment(double v, double c);
    void     freeSegment(Segment* seg);

  private:
	int        segCount;     // number of volume segments allocated
	Segment*   freeSeg;      // first unused segment
	MemPool*   memPool;      // memory pool for volume segments
};

#endif // SEGPOOL_H_

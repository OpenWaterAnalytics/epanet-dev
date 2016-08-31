/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "pattern.h"
#include "Utilities/mempool.h"

#include <limits>
using namespace std;

//-----------------------------------------------------------------------------

// Pattern Factory

Pattern* Pattern::factory(int type_, string name_, MemPool* memPool)
{
    switch ( type_ )
    {
    case FIXED_PATTERN:
        return new(memPool->alloc(sizeof(FixedPattern))) FixedPattern(name_);
        break;
    case VARIABLE_PATTERN:
        return new(memPool->alloc(sizeof(VariablePattern))) VariablePattern(name_);
        break;
    default:
        return nullptr;
    }
}

//-----------------------------------------------------------------------------

// Abstract Pattern Constructor

Pattern::Pattern(string name_, int type_) :
    Element(name_),
    type(type_),
    currentIndex(0),
    interval(0)
{}

Pattern::~Pattern()
{
    factors.clear();
}

//-----------------------------------------------------------------------------

//  Returns a Pattern's factor value at the current time period.

double Pattern::currentFactor()
{
    if ( factors.size() == 0 ) return 1.0;
    return factors[currentIndex];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// Fixed Pattern Constructor/Destructor

FixedPattern::FixedPattern(string name_) :
    Pattern(name_, FIXED_PATTERN),
    startTime(0)
{}

FixedPattern::~FixedPattern() {}

//-----------------------------------------------------------------------------

//  Initializes the state of a Fixed Pattern.

void FixedPattern::init(int intrvl, int tStart)
{
    startTime = tStart;
    if ( interval == 0 ) interval = intrvl;
    if ( factors.size() == 0 ) factors.push_back(1.0);
    int nPeriods = factors.size();
    if ( interval > 0 )
    {
        currentIndex = (startTime/interval) % nPeriods;
    }
}

//-----------------------------------------------------------------------------

//  Finds the time (sec) until the next change in a FixedPattern.

int FixedPattern::nextTime(int t)
{
    int nPeriods = (startTime + t) / interval;
    return (nPeriods + 1) * interval;
}

//-----------------------------------------------------------------------------

//  Advances a FixedPattern to the period associated with time t (sec).

void FixedPattern::advance(int t)
{
    int nPeriods = (startTime + t) / interval;
    currentIndex = nPeriods % factors.size();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//  Variable Pattern Constructor/Destructor

VariablePattern::VariablePattern(string name_) :
    Pattern(name_, VARIABLE_PATTERN)
{}

VariablePattern::~VariablePattern()
{
    times.clear();
}

//-----------------------------------------------------------------------------

//  Initializes the state of a VariablePattern.
//  (Variable patterns have no initial offset time.)

void VariablePattern::init(int intrvl, int tstart)
{
    if ( factors.size() == 0 )
    {
        factors.push_back(1.0);
        times.push_back(0);
    }
    currentIndex = 0;
}

//-----------------------------------------------------------------------------

//  Finds the time (sec) until the next change in a VariablePattern.

int VariablePattern::nextTime(int t)
{
    if ( currentIndex == (int)times.size()-1 )
    {
        return numeric_limits<int>::max();
    }
    return times[currentIndex + 1];
}

//-----------------------------------------------------------------------------

//  Advances a VariablePattern to the period associated with time t (sec).

void VariablePattern::advance(int t)
{
    for (unsigned int i = currentIndex+1; i < times.size(); i++)
    {
        if ( t < times[i] )
        {
            currentIndex = i-1;
            return;
        }
    }
    currentIndex = times.size() - 1;
}

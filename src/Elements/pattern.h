/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file pattern.h
//! \brief Describes the Pattern class and its subclasses.

#ifndef PATTERN_H_
#define PATTERN_H_

#include "Elements/element.h"

#include <string>
#include <vector>

class MemPool;

//! \class Pattern
//! \brief A set of multiplier factors associated with points in time.
//!
//! Time patterns are multipliers used to adjust nodal demands,
//! pump/valve settings, or water quality source inputs over time.
//! Pattern is an abstract class from which the FixedPattern and
//! VariablePattern classes are derived.

class Pattern: public Element
{
  public:

    enum PatternType {FIXED_PATTERN, VARIABLE_PATTERN};

    // Constructor/Destructor
    Pattern(std::string name_, int type_);
    virtual ~Pattern();

    // Pattern factory
    static  Pattern* factory(int type_, std::string name_, MemPool* memPool);

    // Methods
    void           setTimeInterval(int t) { interval = t; }
    void           addFactor(double f) { factors.push_back(f); }
    int            timeInterval() { return interval; }
    int            size() { return factors.size(); }
    double         factor(int i) { return factors[i]; }
    double         currentFactor();
    virtual void   init(int intrvl, int tstart) = 0;
    virtual int    nextTime(int t) = 0;
    virtual void   advance(int t) = 0;

    // Properties
    int            type;                //!< type of time pattern

  protected:
    std::vector<double> factors;        //!< sequence of multiplier factors
    int                 currentIndex;   //!< index of current pattern interval
    int                 interval;       //!< fixed time interval (sec)
};

//------------------------------------------------------------------------------

//! \class FixedPattern
//! \brief A Pattern where factors change at fixed time intervals.
//! \note A fixed pattern wraps around once time exceeds the period
//!       associated with the last multiplier factor supplied.

class FixedPattern : public Pattern
{
  public:

    // Constructor/Destructor
    FixedPattern(std::string name_);
    ~FixedPattern();

    // Methods
    void   init(int intrvl, int tstart);
    int    nextTime(int t);
    void   advance(int t);

  private:
    int    startTime;   //!< offset from time 0 when the pattern begins (sec)
};

//------------------------------------------------------------------------------

//! \class VariablePattern
//! \brief A Pattern where factors change at varying time intervals.
//! \note When time exceeds the last time interval of a variable pattern
//!       the multiplier factor remains constant at its last value.

class VariablePattern : public Pattern
{
  public:

    // Constructor/Destructor
    VariablePattern(std::string name_);
    ~VariablePattern();

    // Methods
    void   addTime(int t) { times.push_back(t); }
    int    time(int i) { return times[i]; }
    void   init(int intrvl, int tstart);
    int    nextTime(int t);
    void   advance(int t);

  private:
    std::vector<int>   times;  //!< times (sec) at which factors change
};

#endif

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file demand.h
//! \brief Describes the Demand class.

#ifndef DEMAND_H_
#define DEMAND_H_

//! \class Demand
//! \brief Specifies the rate of consumer water demand at a Junction node.
//!

class Network;
class Pattern;

class Demand
{
  public:

    Demand();
    ~Demand();

    double   getFullDemand(double multiplier, double patternFactor);

    double   baseDemand;          //!< baseline demand flow (cfs)
    double   fullDemand;          //!< pattern adjusted demand flow (cfs)
    Pattern* timePattern;         //!< time pattern used to adjust baseline demand
};

#endif

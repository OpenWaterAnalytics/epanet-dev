/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "demand.h"
#include "pattern.h"

//-----------------------------------------------------------------------------

//  Demand Constructor

Demand::Demand() :
    baseDemand(0.0),
    fullDemand(0.0),
    timePattern(nullptr)
{
}

//-----------------------------------------------------------------------------

//  Demand Destructor

Demand::~Demand() {}

//-----------------------------------------------------------------------------

//    Find pattern-adjusted full demand

double Demand::getFullDemand(double multiplier, double patternFactor)
{
    if ( timePattern ) patternFactor = timePattern->currentFactor();
    fullDemand = multiplier * baseDemand * patternFactor;
    return fullDemand;
}

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "reservoir.h"
#include "pattern.h"
#include "Core/network.h"

using namespace std;

//-----------------------------------------------------------------------------

//    Constructor

Reservoir::Reservoir(string name_):
    Node(name_),
    headPattern(0)
{
    fullDemand = 0.0;
    fixedGrade = true;
}

Reservoir::~Reservoir() {}

//-----------------------------------------------------------------------------

void Reservoir::convertUnits(Network* nw)
{
    elev /= nw->ucf(Units::LENGTH);
    initQual /= nw->ucf(Units::CONCEN);
}

//-----------------------------------------------------------------------------

void Reservoir::setFixedGrade()
{
    double f = 1.0;
    if ( headPattern )
    {
        f = headPattern->currentFactor();
    }
    head = elev * f;
    fixedGrade = true;
}

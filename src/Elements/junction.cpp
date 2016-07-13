/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

#include "junction.h"
#include "emitter.h"
#include "Core/network.h"
#include "Core/constants.h"
#include "Models/demandmodel.h"

using namespace std;

//-----------------------------------------------------------------------------
//    Junction Constructor
//-----------------------------------------------------------------------------
Junction::Junction(string name_):
    Node(name_),
    pMin(MISSING),
    pFull(MISSING),
    emitter(nullptr)
{
}


//-----------------------------------------------------------------------------
//    Junction Destructor
//-----------------------------------------------------------------------------
Junction::~Junction()
{
    demands.clear();
    delete emitter;
}


//-----------------------------------------------------------------------------
//    Convert junction properties from user's input units to internal units
//    (called after loading network data from an input file)
//-----------------------------------------------------------------------------
void Junction::convertUnits(Network* nw)
{
    // ... convert elevation & initial quality units

    elev /= nw->ucf(Units::LENGTH);
    initQual /= nw->ucf(Units::CONCEN);

    // ... if no demand categories exist, add primary demand to list

    if (demands.size() == 0) demands.push_back(primaryDemand);

    // ... convert flow units for base demand in each demand category

    double qcf = nw->ucf(Units::FLOW);
    for (Demand& demand : demands)
    {
        demand.baseDemand /= qcf;
    }

    // ... convert emitter flow units

    if (emitter) emitter->convertUnits(nw);

    // ... use global pressure limits if no local limits assigned

    if ( pMin == MISSING )  pMin = nw->option(Options::MINIMUM_PRESSURE);
    if ( pFull == MISSING ) pFull = nw->option(Options::SERVICE_PRESSURE);

    // ... convert units of pressure limits

    double pUcf = nw->ucf(Units::PRESSURE);
    pMin /= pUcf;
    pFull /= pUcf;
}


//-----------------------------------------------------------------------------
//    Initialize a junction's properties
//-----------------------------------------------------------------------------
void Junction::initialize(Network* nw)
{
    head = elev + (pFull - pMin) / 2.0;;
    quality = initQual;
    actualDemand = 0.0;
    outflow = 0.0;
    fixedGrade = false;
}


//-----------------------------------------------------------------------------
//    Find a junction's full demand
//-----------------------------------------------------------------------------
void Junction::findFullDemand(double multiplier, double patternFactor)
{
    fullDemand = 0.0;
    for (Demand demand : demands)
    {
        fullDemand += demand.getFullDemand(multiplier, patternFactor);
    }
    actualDemand = fullDemand;
}


//-----------------------------------------------------------------------------
//    Find a junction's actual demand flow and its derivative w.r.t. head
//-----------------------------------------------------------------------------
double Junction::findActualDemand(Network* nw, double h, double &dqdh)
{
    return nw->demandModel->findDemand(this, h-elev, dqdh);
}


//-----------------------------------------------------------------------------
//    Determine if there is not enough pressure to supply junction's demand
//-----------------------------------------------------------------------------
bool Junction::isPressureDeficient(Network* nw)
{
    return nw->demandModel->isPressureDeficient(this);
}


//-----------------------------------------------------------------------------
//    Find the outflow from a junction's emitter
//-----------------------------------------------------------------------------

double Junction::findEmitterFlow(double h, double& dqdh)
{
    dqdh = 0.0;
    if ( emitter) return emitter->findFlowRate(h-elev, dqdh);
    return 0;
}

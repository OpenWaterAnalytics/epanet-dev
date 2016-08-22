/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "pipe.h"
#include "Core/network.h"
#include "Core/constants.h"
#include "Models/headlossmodel.h"
#include "Models/leakagemodel.h"

#include <cmath>
using namespace std;

//-----------------------------------------------------------------------------

Pipe::Pipe(string name) :
    Link(name),
    hasCheckValve(false),
    length(0.0),
    roughness(0.0),
    resistance(0.0),
    lossFactor(0.0),
    leakCoeff1(MISSING),
    leakCoeff2(MISSING),
    bulkCoeff(MISSING),
    wallCoeff(MISSING),
    massTransCoeff(0.0)
{}

Pipe::~Pipe() {}

//-----------------------------------------------------------------------------

void Pipe::convertUnits(Network* nw)
{
    diameter /= nw->ucf(Units::DIAMETER);
    length   /= nw->ucf(Units::LENGTH);

    // ... convert minor loss coeff. from V^2/2g basis to Q^2 basis

    lossFactor = 0.02517 * lossCoeff / pow(diameter, 4);

    // ... convert roughness length units of Darcy-Weisbach headloss model
    //     (millifeet or millimeters to feet)

    if ( nw->option(Options::HEADLOSS_MODEL ) == "D-W")
    {
        roughness = roughness / nw->ucf(Units::LENGTH) / 1000.0;
    }

    // ... apply global default leakage coeffs.

    if ( leakCoeff1 == MISSING ) leakCoeff1 = nw->option(Options::LEAKAGE_COEFF1);
    if ( leakCoeff2 == MISSING ) leakCoeff2 = nw->option(Options::LEAKAGE_COEFF2);

    // ... apply global default reaction coeffs.

    if ( bulkCoeff == MISSING ) bulkCoeff = nw->option(Options::BULK_COEFF);
    if ( wallCoeff == MISSING ) wallCoeff = nw->option(Options::WALL_COEFF);
}

//-----------------------------------------------------------------------------

bool Pipe::isReactive()
{
    if ( bulkCoeff != 0.0 ) return true;
    if ( wallCoeff != 0.0 ) return true;
    return false;
}

//-----------------------------------------------------------------------------

void Pipe::setResistance(Network* nw)
{
    nw->headLossModel->setResistance(this);
}

//-----------------------------------------------------------------------------

void Pipe::setInitStatus(int s)
{
    initStatus = s;
}

//-----------------------------------------------------------------------------

void Pipe::setInitSetting(double s)
{
    if ( s == 0.0 ) initStatus = LINK_CLOSED;
    else            initStatus = LINK_OPEN;
}

//-----------------------------------------------------------------------------

void Pipe::setInitFlow()
{
    // ... flow at velocity of 1 ft/s
    flow = PI * diameter * diameter / 4.0;
}

//-----------------------------------------------------------------------------

double Pipe::getVelocity()
{
    double area = PI * diameter * diameter / 4.0;
    return abs(flow) / area;
}

//-----------------------------------------------------------------------------

double Pipe::getUnitHeadLoss()
{
    if ( length > 0.0 ) return abs(hLoss) * 1000.0 / length;
    return 0.0;
}

//-----------------------------------------------------------------------------

double Pipe::getRe(const double q, const double viscos)
{
    return  abs(q) / (PI * diameter * diameter / 4.0) * diameter / viscos;
}

//-----------------------------------------------------------------------------

void Pipe::findHeadLoss(Network* nw, double q)
{
    if ( status == LINK_CLOSED || status == TEMP_CLOSED )
    {
        HeadLossModel::findClosedHeadLoss(q, hLoss, hGrad);
    }
    else
    {
        nw->headLossModel->findHeadLoss(this, q, hLoss, hGrad);
        if ( hasCheckValve ) HeadLossModel::addCVHeadLoss(q, hLoss, hGrad);
    }
}

//-----------------------------------------------------------------------------

double Pipe::findLeakage(Network* nw, double h, double& dqdh)
{
    return nw->leakageModel->findFlow(leakCoeff1, leakCoeff2, length, h, dqdh);
}


//-----------------------------------------------------------------------------

bool Pipe::changeStatus(int s, bool makeChange, const string reason, ostream& msgLog)
{
    if ( status != s )
    {
        if ( makeChange )
        {
            msgLog << "\n    " << reason;
            status = s;
        }
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

// For debugging only

void Pipe::validateStatus(Network* nw, double qTol)
{
    if ( hasCheckValve && flow < -qTol )
    {
        nw->msgLog << "\nCV " << name << " flow = " << flow*nw->ucf(Units::FLOW);
    }
}

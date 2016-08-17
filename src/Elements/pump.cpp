/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "pump.h"
#include "pattern.h"
#include "Core/constants.h"
#include "Core/network.h"
#include "Core/error.h"
#include "Models/headlossmodel.h"

using namespace std;

//-----------------------------------------------------------------------------

Pump::Pump(string name_) :
    Link(name_),
    speed(1.0),
    speedPattern(nullptr),
    efficCurve(nullptr),
    costPattern(nullptr),
    costPerKwh(0.0)
{
}

Pump::~Pump() {}

//-----------------------------------------------------------------------------

void Pump::convertUnits(Network* nw)
{
    pumpCurve.horsepower /= nw->ucf(Units::POWER);
}

//-----------------------------------------------------------------------------

void Pump::validate(Network* nw)
{
    if ( pumpCurve.curve || pumpCurve.horsepower > 0.0 )
    {
        int err = pumpCurve.setupCurve(nw);
        if ( err ) throw NetworkError(err, name);
    }
    else throw NetworkError(NetworkError::NO_PUMP_CURVE, name);
}

//-----------------------------------------------------------------------------

void Pump::setInitStatus(int s)
{
    initStatus = s;
    if (s == LINK_OPEN)   initSetting = 1.0;
    if (s == LINK_CLOSED) initSetting = 0.0;
}

//-----------------------------------------------------------------------------

void Pump::setInitSetting(double s)
{
    initSetting = s;
    speed = s;
    if ( s <= 0.0 ) initStatus = LINK_CLOSED;
    else            initStatus = LINK_OPEN;
}

//-----------------------------------------------------------------------------

void Pump::setInitFlow()
{
    // ... initial flow is design point of pump curve
    flow = pumpCurve.qInit * initSetting;
}

//-----------------------------------------------------------------------------

void Pump::findHeadLoss(Network* nw, double q)
{
    // --- use high resistance head loss if pump is shut down
    if ( speed == 0.0  || status == LINK_CLOSED || status == TEMP_CLOSED )
    {
        HeadLossModel::findClosedHeadLoss(q, hLoss, hGrad);
    }

    // --- get head loss from pump curve and add a check valve
    //     head loss in case of reverse flow
    else
    {
        pumpCurve.findHeadLoss(speed, q, hLoss, hGrad);
        if ( !isHpPump() ) HeadLossModel::addCVHeadLoss(q, hLoss, hGrad);
    }
}

//-----------------------------------------------------------------------------

double Pump::updateEnergyUsage(Network* nw, int dt)
{
    return pumpEnergy.updateEnergyUsage(this, nw, dt);
}

//-----------------------------------------------------------------------------

bool Pump::changeStatus(int s, bool makeChange, const string reason, ostream& msgLog)
{
    if ( status != s )
    {
        if ( makeChange )
        {
            if ( s == LINK_OPEN && speed == 0.0 ) speed = 1.0;
            if ( s == LINK_CLOSED ) flow = ZERO_FLOW;
            msgLog << "\n    " << reason;
            status = s;
        }
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

bool Pump::changeSetting(double s, bool makeChange, const string reason, ostream& msgLog)
{
    if ( speed != s )
    {
        if ( status == Link::LINK_CLOSED && s == 0.0 )
        {
            speed = s;
            return false;
        }

        if ( makeChange )
        {
            if ( s == 0.0 )
            {
                status = Link::LINK_CLOSED;
                flow = ZERO_FLOW;
            }
            else status = Link::LINK_OPEN;
            speed = s;
            msgLog << "\n    " << reason;
        }
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

////  FOR DEBUGGING ONLY

void Pump::validateStatus(Network* nw, double qTol)
{
    if (flow < -qTol)
    {
        nw->msgLog << "\nPump " << name << " flow = " << flow*nw->ucf(Units::FLOW);
    }
}

//-----------------------------------------------------------------------------

void Pump::applyControlPattern(ostream& msgLog)
{
    if ( speedPattern )
    {
        changeSetting(speedPattern->currentFactor(), true, "speed pattern", msgLog);
    }
}

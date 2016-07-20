/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "tankmixmodel.h"
#include "Core/error.h"
#include "Models/qualmodel.h"
#include "Elements/tank.h"
#include "Utilities/segpool.h"

#include <cmath>
#include <algorithm>

using namespace std;

const char* TankMixModel::MixingModelWords[] = {"MIXED", "2COMP", "FIFO", "LIFO", 0};

//-----------------------------------------------------------------------------

TankMixModel::TankMixModel() :
    type(MIX1),
    cTol(0.0),
    fracMixed(0.0),
    cTank(0.0),
    vMixed(0.0),
    firstSeg(nullptr),
    lastSeg(nullptr)
{ }

TankMixModel::~TankMixModel()
{ }

//-----------------------------------------------------------------------------

//  Initialize a tank's mixing model.

void TankMixModel::init(Tank* tank, SegPool* segPool, double _cTol)
{
    // ... save project's quality tolerance, initial quality, and
    //     mixing zone volume (needed only for MIX2 model)
    cTol = _cTol;
    cTank = tank->quality;
    vMixed = fracMixed * tank->maxVolume;

    // ... create a volume segment for the entire tank
    lastSeg = nullptr;
    firstSeg = segPool->getSegment(tank->volume, cTank);
    if ( firstSeg == nullptr )
        throw SystemError(SystemError::OUT_OF_MEMORY);

    // ... create a second segment for the 2-compartment model
    if ( type == MIX2 )
    {
        // ... firstSeg contains stagnant zone
        double v = max(0.0, tank->volume - vMixed);
        firstSeg->v = v;

        // ... lastSeg contains mixing zone
        lastSeg = segPool->getSegment(tank->volume - v, cTank);
        if ( lastSeg == nullptr )
            throw SystemError(SystemError::OUT_OF_MEMORY);
        firstSeg->next = lastSeg;
    }
}

//-----------------------------------------------------------------------------

//  Update the quality of water released by a tank.

double TankMixModel::findQuality(double vNet, double vIn,
                                 double wIn, SegPool* segPool)
{
	switch (type)
	{
    case MIX2: return findMIX2Quality(vNet, vIn, wIn);
    case FIFO: return findFIFOQuality(vNet, vIn, wIn, segPool);
    case LIFO: return findLIFOQuality(vNet, vIn, wIn, segPool);
    default:   return findMIX1Quality(vNet, vIn, wIn);
	}
}

//-----------------------------------------------------------------------------

//  React the contents of a tank.

double TankMixModel::react(Tank* tank, QualModel* qualModel, double tstep)
{
    double massReacted = 0.0;
    Segment* seg = firstSeg;
    while (seg)
    {
        double c = seg->c;
        seg->c = qualModel->tankReact(tank, c, tstep);
        massReacted += (c - seg->c) * seg->v;
        seg = seg->next;
    }
    return massReacted;
}

//-----------------------------------------------------------------------------

//  Find the water quality mass stored in a tank.

double TankMixModel::storedMass()
{
    double totalMass = 0.0;
    Segment* seg = firstSeg;
    while (seg)
    {
        totalMass += seg->c * seg->v;
        seg = seg->next;
    }
    return totalMass;
}

//-----------------------------------------------------------------------------

//  Find the quality released from a completely mixed tank.

double TankMixModel::findMIX1Quality(double vNet, double vIn, double wIn)
{
    double vNew = firstSeg->v + vIn;
    if ( vNew > 0.0 )
    {
        firstSeg->c = (firstSeg->c * firstSeg->v + wIn) / vNew;
    }
    firstSeg->v += vNet;
    cTank = firstSeg->c;
    return cTank;
}

//-----------------------------------------------------------------------------

//  Find the quality released from the mixing zone of a 2-compartment tank.

double TankMixModel::findMIX2Quality(double vNet, double vIn, double wIn)
{
    Segment* mixZone = lastSeg;    // mixing compartment
    Segment* stagZone = firstSeg;  // stagnant compartment
    double vTransfer = 0.0;        // volume transferred between compartments

    // ... tank is filling
    if ( vNet > 0.0 )
    {
        vTransfer = max(0.0, mixZone->v + vNet - vMixed);
        if ( vIn > 0.0 )
        {
            mixZone->c = (mixZone->c * mixZone->v + wIn) / (mixZone->v + vIn);
        }
        if ( vTransfer > 0.0 )
        {
            stagZone->c = (stagZone->c * stagZone->v + mixZone->c * vTransfer) /
                          (stagZone->v + vTransfer);
        }
    }

    // ... tank is emptying
    if ( vNet < 0.0 )
    {
        if ( stagZone->v > 0.0 )
        {
            vTransfer = min(stagZone->v, -vNet);
        }
        if ( vIn + vTransfer > 0.0 )
        {
            mixZone->c = (mixZone->c * mixZone->v + wIn +
                          stagZone->c * vTransfer) /
                         (mixZone->v + vIn + vTransfer);
        }
    }

    // ... update compartment volumes
    if ( vTransfer > 0.0 )
    {
        mixZone->v = vMixed;
        if ( vNet > 0.0 ) stagZone->v += vTransfer;
        else              stagZone->v = max(0.0, stagZone->v - vTransfer);
    }
    else
    {
        mixZone->v += vNet;
        mixZone->v = min(mixZone->v, vMixed);
        mixZone->v = max(0.0, mixZone->v);
        stagZone->v = 0.0;
    }

    // ... mixing zone quality is what leaves the tank
    cTank = mixZone->c;
    return cTank;
}

//-----------------------------------------------------------------------------

//  Find the quality leaving the the first segment of a plug flow (FIFO) tank.

double TankMixModel::findFIFOQuality(double vNet, double vIn, double wIn,
                                     SegPool* segPool)
{
    // ... add new last segment for flow entering the tank
    if ( vIn > 0.0 )
    {
        // ... increase segment volume if inflow has same quality as segment
        double cIn = wIn / vIn;
        if ( lastSeg && abs(lastSeg->c - cIn ) < cTol ) lastSeg->v += vIn;

        // ... otherwise add a new last segment to the tank
        else
        {
            Segment* seg = segPool->getSegment(vIn, cIn);
            if ( seg == nullptr ) throw SystemError(SystemError::OUT_OF_MEMORY);
            if ( firstSeg == nullptr ) firstSeg = seg;
            if ( lastSeg ) lastSeg->next = seg;
            lastSeg = seg;
        }
    }

    // ... withdraw flow from first segment
    double vSum = 0.0;
    double wSum = 0.0;
    double vOut = vIn - vNet;
    while (vOut > 0.0)
    {
        Segment* seg = firstSeg;
        if ( seg == nullptr ) break;
        double vSeg = min(seg->v, vOut);
        if ( seg == lastSeg ) vSeg = vOut;
        vSum += vSeg;
        wSum += seg->c * vSeg;
        vOut -= vSeg;
        if ( vOut >= 0.0 && vSeg >= seg->v && seg->next )
        {
            firstSeg = seg->next;
            segPool->freeSegment(seg);
        }
        else seg->v -= vSeg;
    }

    // ... return average quality withdrawn from 1st segment
    if ( vSum > 0.0 ) cTank = wSum / vSum;
    else if ( firstSeg == nullptr ) cTank = 0.0;
    else  cTank = firstSeg->c;
    return cTank;
}

//-----------------------------------------------------------------------------

//  Find the quality leaving the bottom (last) segment of a vertical FIFO tank.

double TankMixModel::findLIFOQuality(double vNet, double vIn, double wIn,
                                     SegPool* segPool)
{
    // ... if filling then create a new first segment
    if ( vNet > 0.0 )
    {
        // ... increase current first segment volume if inflow has same quality
        double cIn = wIn / vIn;
        if ( firstSeg && abs(firstSeg->c - cIn ) < cTol ) firstSeg->v += vNet;

        // ... otherwise add a new first segment to the tank
        else
        {
            Segment* seg = segPool->getSegment(vNet, cIn);
            if ( seg == nullptr ) throw SystemError(SystemError::OUT_OF_MEMORY);
            seg->next = firstSeg;
            firstSeg = seg;
        }
        cTank = firstSeg->c;
    }

    // ... if emptying then remove first segments until vNet is reached
    else if ( vNet < 0.0 )
    {
        double vSum = 0.0;
        double wSum = 0.0;
        vNet = -vNet;
        while ( vNet > 0.0 )
        {
            Segment* seg = firstSeg;
            if ( seg == nullptr ) break;
            double vSeg = min(seg->v, vNet);
            if ( seg->next == nullptr ) vSeg = vNet;
            vSum += vSeg;
            wSum += seg->c * vSeg;
            vNet -= vSeg;
            if ( vNet >= 0.0 && vSeg >= seg->v && seg->next )
            {
                firstSeg = seg->next;
                segPool->freeSegment(seg);
            }
            else seg->v -= vSeg;
        }

        // ... avg. quality released is mixture of quality in flow
        //     released and any inflow
        cTank = (wSum + wIn) / (vSum + vIn);
    }
    return cTank;
 }

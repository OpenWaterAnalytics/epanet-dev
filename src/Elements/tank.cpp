/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "tank.h"
#include "curve.h"
#include "Core/network.h"
#include "Core/constants.h"
#include "Core/error.h"

#include <algorithm>
using namespace std;

//-----------------------------------------------------------------------------

//  Tank Constructor

Tank::Tank(string name_) :
    Node(name_),
    initHead(0.0),
    minHead(0.0),
    maxHead(0.0),
    diameter(0.0),
    minVolume(0.0),
    bulkCoeff(MISSING),
    volCurve(nullptr),
    maxVolume(0.0),
    volume(0.0),
    area(0.0),
    ucfLength(1.0),
    pastHead(0.0),
    pastVolume(0.0),
    pastOutflow(0.0)
{
    fullDemand = 0.0;
    fixedGrade = true;
}

//-----------------------------------------------------------------------------

//  Convert user's input units to internal units

void Tank::convertUnits(Network* nw)
{

    // ... convert from user to internal units
    ucfLength = nw->ucf(Units::LENGTH);
    initHead /= ucfLength;
    minHead /= ucfLength;
    maxHead /= ucfLength;
    diameter /= ucfLength;
    area = PI * diameter * diameter / 4.0;
    elev /= ucfLength;
    minVolume /= nw->ucf(Units::VOLUME);
    initQual /= nw->ucf(Units::CONCEN);

    // ... assign default bulk reaction rate coeff.
    if ( bulkCoeff == MISSING ) bulkCoeff = nw->option(Options::BULK_COEFF);
}

//-----------------------------------------------------------------------------

//  Check that tank has valid data

void Tank::validate(Network* nw)
{
    // ... check for enough info to compute volume
    if ( diameter == 0.0 && volCurve == nullptr )
    {
        throw NetworkError(NetworkError::INVALID_VOLUME_CURVE, name);
    }

    // ... check that volume curve (depth v. volume in user units)
    //     covers range of depth limits
    if ( volCurve )
    {
        if ( volCurve->size() < 2 )
        {
            throw NetworkError(NetworkError::INVALID_VOLUME_CURVE, name);
        }
        double tankHead = volCurve->x(0) / ucfLength + elev;
        minHead = max(minHead, tankHead);
        tankHead = volCurve->x(volCurve->size() - 1) / ucfLength + elev;
        maxHead = min(maxHead, tankHead);
     }

    // ... check for consistent depth limits
    if ( maxHead < minHead )
    {
        throw NetworkError(NetworkError::INVALID_TANK_LEVELS, name);
    }
    initHead = max(initHead, minHead);
    initHead = min(initHead, maxHead);
}

//-----------------------------------------------------------------------------

//  Initialize state of tank

void Tank::initialize(Network* nw)
{
    head = initHead;
    pastHead = initHead;
    outflow = 0.0;
    pastOutflow = 0.0;
    quality = initQual;
    updateArea();
    if ( volCurve ) minVolume = findVolume(minHead);
    else if ( minVolume == 0.0 ) minVolume = (minHead - elev) * area;
    volume = findVolume(head);
    maxVolume = findVolume(maxHead);
    fixedGrade = true;
}

//-----------------------------------------------------------------------------

//  Compute tank volume from water surface elevation

double Tank::findVolume(double aHead)
{
    // ... convert head to water depth

    double depth = aHead - elev;

    // ... tank has a volume curve (in original user units)

    if ( volCurve )
    {
        // ... find slope and intercept of curve segment containing depth

        depth *= ucfLength;
        double slope, intercept;
        volCurve->findSegment(depth, slope, intercept);

        // ... compute volume and convert to ft3

        double ucfArea = ucfLength * ucfLength;
        return (slope * depth + intercept) / (ucfArea * ucfLength);
    }

    // ... tank is cylindrical

    if ( minVolume > 0.0 ) depth = max(aHead - minHead, 0.0);
    return minVolume + area * depth;
}

//-----------------------------------------------------------------------------

//  Compute tank surface area from water depth

void Tank::updateArea()
{

    // ... tank has a volume curve (in original user units)

    if ( volCurve )
    {
        // ... find slope of curve segment containing depth

        double slope, intercept;
        double depth = head - elev;
        volCurve->findSegment(depth*ucfLength, slope, intercept);

        // ... curve segment slope (dV/dy) is avg. area over interval;
        //     convert to internal units

        area = slope / ucfLength / ucfLength;
    }

    // ... area of cylindrical tank remains constant
}

//-----------------------------------------------------------------------------

//  Compute water surface elevation from tank volume

double Tank::findHead(double aVolume)
{
    // ... tank has a volume curve (in original user units)

    if ( volCurve )
    {
        double ucfArea = ucfLength * ucfLength;
        aVolume *= ucfArea * ucfLength;
        return elev + volCurve->getXofY(aVolume) / ucfLength;
    }

    // ... tank is cylindrical

    else
    {
        aVolume = max(0.0, aVolume - minVolume);
        return minHead + aVolume / area;
    }
}

//-----------------------------------------------------------------------------

//  Update tank volume after a given time step

void Tank::updateVolume(int tstep)
{
    // ... new volume based on current outflow

    volume += outflow * tstep;

    // ... check if min/max levels reached within an additional 1 second of flow

    double v1 = volume + outflow;
    if ( v1 <= minVolume )
    {
        volume = minVolume;
        head = minHead;
    }
    else if ( v1 >= maxVolume )
    {
        volume = maxVolume;
        head = maxHead;
    }

    // ... find head at new volume

    else head = findHead(volume);
 }

//-----------------------------------------------------------------------------

//  Find time to fill (or empty) tank to a given volume

int Tank::timeToVolume(double v)
{
    // ... make sure target volume is within bounds

    v = max(v, minVolume);
    v = min(v, maxVolume);

    // ... make sure outflow is positive for filling or negative for emptying

    if ( (v-volume) * outflow  <= 0.0 ) return -1;

    // ... required time is volume change over outflow rate

    double t = (v - volume) / outflow;
    return (int) (t + 0.5);
}

//-----------------------------------------------------------------------------

//  Check if there is inflow to a full tank or outflow from an empty one

bool Tank::isClosed(double flow)
{
    if ( !fixedGrade ) return false;
    if ( head >= maxHead && flow < 0.0 ) return true;
    if ( head <= minHead && flow > 0.0 ) return true;
    return false;
}

//-----------------------------------------------------------------------------

//  Find fixed grade water surface elevation

void Tank::setFixedGrade()
{
    fixedGrade = true;
    //head = findHead(volume);
}

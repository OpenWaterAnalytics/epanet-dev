/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "link.h"
#include "pipe.h"
#include "pump.h"
#include "valve.h"
#include "Core/constants.h"
#include "Core/network.h"
#include "Utilities/mempool.h"

#include <cmath>
#include <algorithm>
using namespace std;

//-----------------------------------------------------------------------------

static const string s_From = " status changed from ";
static const string s_To =   " to ";
static const string linkStatusWords[] = {"CLOSED", "OPEN", "ACTIVE", "TEMP_CLOSED"};

//-----------------------------------------------------------------------------

/// Constructor

Link::Link(string name_) :
    Element(name_),
    rptFlag(false),
    fromNode(nullptr),
    toNode(nullptr),
    initStatus(LINK_OPEN),
    diameter(0.0),
    lossCoeff(0.0),
    initSetting(1.0),
    status(0),
    flow(0.0),
    leakage(0.0),
    hLoss(0.0),
    hGrad(0.0),
    setting(0.0),
    quality(0.0)
{}

/// Destructor

Link::~Link()
{}

//-----------------------------------------------------------------------------

/// Factory Method

Link* Link::factory(int type_, string name_, MemPool* memPool)
{
    switch (type_)
    {
    case PIPE:
        return new(memPool->alloc(sizeof(Pipe))) Pipe(name_);
        break;
    case PUMP:
        return new(memPool->alloc(sizeof(Pump))) Pump(name_);
        break;
    case VALVE:
        return new(memPool->alloc(sizeof(Valve))) Valve(name_);
        break;
    default:
        return nullptr;
    }
}

//-----------------------------------------------------------------------------

void Link::initialize(bool reInitFlow)
{
    status = initStatus;
    setting = initSetting;
    if ( reInitFlow )
    {
        if ( status == LINK_CLOSED ) flow = ZERO_FLOW;
        else setInitFlow();
    }
    leakage = 0.0;
}

//-----------------------------------------------------------------------------

double Link::getUnitHeadLoss()
{
    return hLoss;
}

//-----------------------------------------------------------------------------

string Link::writeStatusChange(int oldStatus)
{
    stringstream ss;
    ss << "    " << typeStr() << " " <<
	    name << s_From << linkStatusWords[oldStatus] << s_To <<
	    linkStatusWords[status];
    return ss.str();
}

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

#include "node.h"
#include "junction.h"
#include "reservoir.h"
#include "tank.h"
#include "qualsource.h"
#include "Utilities/mempool.h"

using namespace std;

//-----------------------------------------------------------------------------

// Constructor

Node::Node(string name_) :
    Element(name_),
    rptFlag(false),
    elev(0.0),
    xCoord(-1e20),
    yCoord(-1e20),
    initQual(0.0),
    qualSource(nullptr),
    fixedGrade(false),
    head(0.0),
    qGrad(0.0),
    fullDemand(0.0),
    actualDemand(0.0),
    outflow(0.0),
    quality(0.0)
{}

// Destructor

Node::~Node()
{
    delete qualSource;
}

//-----------------------------------------------------------------------------

// Factory Method

Node* Node::factory(int type_, string name_, MemPool* memPool)
{
    switch (type_)
    {
    case JUNCTION:
        return new(memPool->alloc(sizeof(Junction))) Junction(name_);
        break;
    case RESERVOIR:
        return new(memPool->alloc(sizeof(Reservoir))) Reservoir(name_);
        break;
    case TANK:
        return new(memPool->alloc(sizeof(Tank))) Tank(name_);
        break;
    default:
        return nullptr;
    }
}

//-----------------------------------------------------------------------------

void Node::initialize(Network* nw)
{
    head = elev;
    quality = initQual;
    if ( qualSource ) qualSource->quality = quality;
    actualDemand = 0.0;
    outflow = 0.0;
    if ( type() == JUNCTION ) fixedGrade = false;
    else fixedGrade = true;
}

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "qualsource.h"
#include "node.h"
#include "pattern.h"
#include "Core/constants.h"

#include <cmath>
#include <algorithm>
using namespace std;

const char* QualSource::SourceTypeWords[] =
        {"CONCEN", "MASS", "FLOWPACED", "SETPOINT", 0};

//-----------------------------------------------------------------------------

QualSource::QualSource() :
    type(CONCEN),
    base(0.0),
    pattern(nullptr),
    strength(0.0),
    outflow(0.0),
    quality(0.0)
{}

QualSource::~QualSource()
{}

//-----------------------------------------------------------------------------

bool QualSource::addSource(Node* node, int t, double b, Pattern* p)
{
    if ( node->qualSource == nullptr )
    {
        node->qualSource = new QualSource();
        if ( node->qualSource == nullptr ) return false;
    }
    node->qualSource->type = t;
    node->qualSource->base = b;
    node->qualSource->pattern = p;
    node->qualSource->quality = 0.0;
    node->qualSource->outflow = 0.0;
    return true;
}

//-----------------------------------------------------------------------------

void QualSource::setStrength(Node* node)
{
    strength = base;
    if ( pattern ) strength *= pattern->currentFactor();
    if ( type == MASS ) strength *= 60.0;         // mass/min -> mass/sec
    else                strength /= FT3perL;      // mass/L -> mass/ft3
}

//-----------------------------------------------------------------------------

double QualSource::getQuality(Node* node)
{
    // ... no source contribution if no flow out of node
    quality = node->quality;
    if ( outflow == 0.0 ) return quality;

    switch (type)
    {
    case CONCEN:
        switch (node->type())
        {
        // ... for junctions, outflow quality is the node's quality plus the
        //     source's quality times the fraction of outflow to the network
        //     contributed by external inflow (i.e., negative demand)
        //     NOTE: qualSource.outflow is flow in links leaving the node,
        //           node->outflow is node's external outflow (demands, etc.)
        case Node::JUNCTION:
            if ( node->outflow < 0.0 )
            {
                quality += strength * (-node->outflow / outflow);
            }
            break;

        // ... for tanks, the outflow quality is the larger of the
        //     tank's value and the source's value
        case Node::TANK:
            quality = max(quality, strength);
            break;

        // ... for reservoirs, outflow quality equals the source strength
        case Node::RESERVOIR:
            quality = strength;
            break;
        }
        break;

    case MASS:
        // ... outflow quality is node's quality plus the source's
        //     mass flow rate divided by the node's outflow to the network
        quality += strength / outflow;
        break;

    case SETPOINT:
        // ... outflow quality is larger of node quality and setpoint strength
         quality = max(quality, strength);
        break;

    case FLOWPACED:
        // ... outflow quality is node's quality + source's strength
        quality += strength;
        break;
    }
    return quality;
}

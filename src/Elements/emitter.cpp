/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

#include "emitter.h"
#include "junction.h"
#include "pattern.h"
#include "Core/network.h"

#include <cmath>
using namespace std;

//-----------------------------------------------------------------------------

// Emitter Constructor

Emitter::Emitter() :
    flowCoeff(0),
    expon(0.0),
    timePattern(0)
{}

//-----------------------------------------------------------------------------

// Emitter Destructor

Emitter::~Emitter() {}

//-----------------------------------------------------------------------------

// Adds or edits a junction's emitter.

bool Emitter::addEmitter(Junction* junc, double c, double e, Pattern* p)
{
    if ( junc->emitter == nullptr )
    {
        junc->emitter = new Emitter();
        if ( junc->emitter == nullptr ) return false;
    }
    junc->emitter->flowCoeff = c;
    junc->emitter->expon = e;
    junc->emitter->timePattern = p;
    return true;
}

//-----------------------------------------------------------------------------

//  Converts an emitter's properties from user units to internal units.

void Emitter::convertUnits(Network* network)
{
    // ... get units conversion factors

    double qUcf = network->ucf(Units::FLOW);
    double pUcf = network->ucf(Units::PRESSURE);

    // ... convert flowCoeff from user flow units per psi (or meter)
    //     to cfs per foot of head

    flowCoeff *= pow(pUcf, expon) / qUcf;
}

//-----------------------------------------------------------------------------

//  Finds the outflow and its derivative from an emitter at a given pressure head.

double Emitter::findFlowRate(double h, double& dqdh)
{
    dqdh = 0.0;
    if ( h <= 0.0 ) return 0.0;
    double a = flowCoeff;
    if (timePattern) a *= timePattern->currentFactor();
    double q = a * pow(h, expon);
    dqdh = expon * q / h;
    return q;
}

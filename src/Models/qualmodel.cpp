/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "qualmodel.h"
#include "Core/network.h"
#include "Core/constants.h"
#include "Elements/pipe.h"
#include "Elements/tank.h"
#include "Elements/qualsource.h"
#include "Utilities/utilities.h"

#include <cmath>
#include <algorithm>
using namespace std;

static const double Small =  1.E-6;
static const double Ctrace = 100.0 * LperFT3;

//-----------------------------------------------------------------------------
// Quality model constructor / destructor
//-----------------------------------------------------------------------------

QualModel::QualModel(int _type) : type(_type)
{
}

QualModel::~QualModel()
{
}

//-----------------------------------------------------------------------------
// Quality model factory
//-----------------------------------------------------------------------------

QualModel* QualModel::factory(const string model)
{
    if ( model == "CHEMICAL" ) return new ChemModel();
    if ( model == "TRACE" )    return new TraceModel();
    if ( model == "AGE" )      return new AgeModel();
    return nullptr;
}


//-----------------------------------------------------------------------------
//  Chemical Quality Model
//-----------------------------------------------------------------------------

ChemModel::ChemModel() : QualModel(CHEM)
{
    reactive = false;       // true if chemical is reactive
    diffus = DIFFUSIVITY;   // chemical's diffusivity (ft2/sec)
    viscos = VISCOSITY;     // water kin. viscosity (ft2/sec)
    Sc = 0.0;               // Schmidt number
    pipeOrder = 1.0;        // pipe bulk fluid reaction order
    tankOrder = 1.0;        // tank bulk fluid reaction order
    wallOrder = 1.0;        // pipe wall reaction order
    pipeUcf = 1.0;          // volume conversion factor for pipes
    tankUcf = 1.0;          // volume conversion factor for tanks
    cLimit = 0.001;         // min/max concentration limit (mass/ft3)
}

//-----------------------------------------------------------------------------

//  Initialize the properties of a Chemical quality model.

void ChemModel::init(Network* nw)
{
    reactive = setReactive(nw);

    // save reaction orders
    pipeOrder = nw->option(Options::BULK_ORDER);
    tankOrder = nw->option(Options::TANK_ORDER);
    wallOrder = nw->option(Options::WALL_ORDER);

    // volume conversion factors for reaction rate expressions
    pipeUcf = pow(LperFT3, (1.0 - pipeOrder));
    tankUcf = pow(LperFT3, (1.0 - tankOrder));

    // save diffusuivity, viscosity & Schmidt number
    diffus = nw->option(Options::MOLEC_DIFFUSIVITY);
    viscos = nw->option(Options::KIN_VISCOSITY);
    if ( diffus > 0.0 ) Sc = viscos / diffus;
    else Sc = 0.0;

    // save concentration limit
    cLimit = nw->option(Options::LIMITING_CONCEN) / FT3perL;
}

//-----------------------------------------------------------------------------

//  Check whether the chemical is reactive or not.

bool ChemModel::setReactive(Network* nw)
{
    for (Link* link : nw->links )
    {
        if ( link->isReactive() ) return true;
    }

    for (Node* node : nw->nodes)
    {
        if ( node->isReactive() ) return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

//  Find a mass transfer coefficient between the bulk flow and the pipe wall
//  for the current flow rate.

void ChemModel::findMassTransCoeff(Pipe* pipe)
{
    massTransCoeff = 0.0;

    // ... return if no wall reaction or zero diffusivity
    if ( pipe->wallCoeff == 0.0 ) return;
    if ( diffus == 0.0 ) return;

    // ... compute Reynolds No.
    double d = pipe->diameter;
    double Re = pipe->getRe(pipe->flow, viscos);

    // ... Sherwood No. for stagnant flow
    //     (mass transfer coeff. = diffus./radius)
    double Sh;
    if ( Re < 1.0 ) Sh = 2.0;

    // ... Sherwood No. for turbulent flow
    //     (from Notter-Sleicher formula) (Cussler formula)
    else if ( Re >= 2300.0 )
    {
        //Sh = 0.0149 * pow(Re, 0.88) * pow(Sc, 0.333);
        Sh = 0.026 * pow(Re, 0.8) * pow(Sc, 1.0/3.0);
    }

    // ... Sherwood No. for laminar flow
    //     (from Graetz solution formula) (Cussler formula)
    else
    {
        //double y = diam / length * Re * Sc;
        //Sh = 3.65 + 0.0668*y / (1.0 + 0.04*pow(y, 0.667));
        double y = Re * Sc * (d / pipe->length);
        Sh = 1.62 * pow(y, 1.0/3.0);
    }

   // ... compute mass transfer coeff. (in ft/sec)
   massTransCoeff = Sh * diffus / d;
}

//-----------------------------------------------------------------------------

//  Update a chemical's pipe concentration after reaction over a given time step.

double ChemModel::pipeReact(Pipe* pipe, double c, double tstep)
{
    double dCdT = 0.0;

    double kb = pipe->bulkCoeff / SECperDAY;
    if ( kb != 0.0 ) dCdT = findBulkRate(kb, pipeOrder, c) * pipeUcf;

    double kw = pipe->wallCoeff / SECperDAY;
    if ( kw != 0.0 ) dCdT += findWallRate(kw, pipe->diameter, wallOrder, c);

    c = c + dCdT * tstep;
    return max(0.0, c);
}

//-----------------------------------------------------------------------------

//  Update a chemical's tank concentration after reaction over a given time step.

double ChemModel::tankReact(Tank* tank, double c, double tstep)
{
    double kb = tank->bulkCoeff / SECperDAY;
    if ( kb == 0.0 ) return c;
    c = c + findBulkRate(kb, tankOrder, c) * tankUcf * tstep;
    return max(0.0, c);
}

//-----------------------------------------------------------------------------

//  Find the bulk reaction rate at a given chemical concentration.

double ChemModel::findBulkRate(double kb, double order, double c)
{
    double c1;

    // ... zero-order kinetics:
    if ( order == 0.0 ) c = 1.0;

    // ... Michaelis-Menton kinetics:
    else if ( order < 0.0 )
    {
       c1 = cLimit + Utilities::sign(kb) * c;
       if ( abs(c1) < Small ) c1 = Utilities::sign(c1) * Small;
       c = c/c1;
    }

    // ... n-th order kinetics:
    else
    {
       // ... account for limiting potential
       if ( cLimit == 0.0 ) c1 = c;
       else c1 = max(0.0, Utilities::sign(kb)*(cLimit - c));

       // ... compute concentration potential
       if ( order == 1.0 ) c = c1;
       else if ( order == 2.0 ) c = c1 * c;
       else c = c1 * pow(max(0.0, c), order-1.0f);
    }

    // ... reaction rate = bulk coeff. * concen. potential

    if ( c < 0.0 ) c = 0.0;
    return kb * c;
}

//-----------------------------------------------------------------------------

//  Find the wall reaction rate at a given chemical concentration.

double ChemModel::findWallRate(double kw, double d, double order, double c)
{
    // ... find pipe's hydraulic radius (area / wetted perimeter)

    if ( d == 0.0 ) return 0.0;
    double rh = d / 4.0;          // hydraulic radius

    // ... if mass transfer ignored return rate based just on wall coeff.

    if ( massTransCoeff == 0.0 )
    {
        if (order == 0.0) c = 1.0;
        return c * kw / rh;
    }

    // ... otherwise rate is combination of wall & mass transfer coeff.

    else
    {
        // ... for 0-order wall reaction, rate is smaller of
        //     wall rate & mass transfer rate
        if ( order == 0.0 )
        {
            double kf = Utilities::sign(kw) * c * massTransCoeff;   // mass/ft2/sec
            if ( abs(kf) < abs(kw) ) kw = kf;
            return kw / rh;                                      // mass/ft3/sec
        }

        // ... for first order reaction, rate is concen. *
        //     composite of wall & mass transfer coeffs.
        else
        {
            double kf = massTransCoeff;
            return c * kw * kf / (kf + abs(kw)) / rh;
        }
    }
}


//-----------------------------------------------------------------------------
//  Trace Quality Model
//-----------------------------------------------------------------------------

void TraceModel::init(Network* nw)
{
    traceNode = nw->node(nw->option(Options::TRACE_NODE));
    traceNode->quality = Ctrace;
}

//-----------------------------------------------------------------------------

double TraceModel::findTracerAdded(Node* node, double qIn)
{
    if ( node != traceNode ) return 0.0;
    double c = Ctrace;   // - node->quality;
    //if ( qIn <= 0.0 ) c = Ctrace;
    //qIn -= node->outflow;
    node->quality = Ctrace;
    return max(0.0, c * qIn);
}


//-----------------------------------------------------------------------------
//  Age Quality Model
//-----------------------------------------------------------------------------
double AgeModel::pipeReact(Pipe* pipe, double age, double tstep)
{
    return age + tstep / 3600.0 * LperFT3;
}

//-----------------------------------------------------------------------------

double AgeModel::tankReact(Tank* tank, double age, double tstep)
{
    return age + tstep / 3600.0 * LperFT3;
}

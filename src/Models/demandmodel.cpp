/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

#include "demandmodel.h"
#include "Elements/junction.h"

#include <cmath>
#include <algorithm>
using namespace std;

//-----------------------------------------------------------------------------
// Parent constructor and destructor
//-----------------------------------------------------------------------------

DemandModel::DemandModel() : expon(0.0)
{}

DemandModel::DemandModel(double expon_) : expon(expon_)
{}

DemandModel::~DemandModel()
{}

//-----------------------------------------------------------------------------
// Demand model factory
//-----------------------------------------------------------------------------

DemandModel* DemandModel::factory(const string model, const double expon_)
{
    if      ( model == "FIXED" )       return new FixedDemandModel();
    else if ( model == "CONSTRAINED" ) return new ConstrainedDemandModel();
    else if ( model == "POWER" )       return new PowerDemandModel(expon_);
    else if ( model == "LOGISTIC" )    return new LogisticDemandModel(expon_);
    else return nullptr;
}

//-----------------------------------------------------------------------------
// Default functions
//-----------------------------------------------------------------------------

double DemandModel::findDemand(Junction* junc, double p, double& dqdh)
{
    dqdh = 0.0;
    return junc->fullDemand;
}


//-----------------------------------------------------------------------------
///  Fixed Demand Model
//-----------------------------------------------------------------------------

FixedDemandModel::FixedDemandModel()
{}


//-----------------------------------------------------------------------------
///  Constrained Demand Model
//-----------------------------------------------------------------------------

ConstrainedDemandModel::ConstrainedDemandModel()
{}

bool ConstrainedDemandModel::isPressureDeficient(Junction* junc)
{
    //if ( junc->fixedGrade ||
    // ... return false if normal full demand is non-positive
    if (junc->fullDemand <= 0.0 ) return false;
    double hMin = junc->elev + junc->pMin;
    if ( junc->head < hMin )
    {
        junc->fixedGrade = true;
        junc->head = hMin;
        return true;
    }
    return false;
}

double ConstrainedDemandModel::findDemand(Junction* junc, double p, double& dqdh)
{
    dqdh = 0.0;
    return junc->actualDemand;
}

//-----------------------------------------------------------------------------
///  Power Demand Model
//-----------------------------------------------------------------------------

PowerDemandModel::PowerDemandModel(double expon_) : DemandModel(expon_)
{}

double PowerDemandModel::findDemand(Junction* junc, double p, double& dqdh)
{
    // ... initialize demand and demand derivative

    double qFull = junc->fullDemand;
    double q = qFull;
    dqdh = 0.0;

    // ... check for positive demand and pressure range

    double pRange = junc->pFull - junc->pMin;
    if ( qFull > 0.0 && pRange > 0.0)
    {
        // ... find fraction of full pressure met (f)

      	double factor = 0.0;
        double f = (p - junc->pMin) / pRange;

        // ... apply power function

        if (f <= 0.0) factor = 0.0;
        else if (f >= 1.0) factor = 1.0;
        else
        {
            factor = pow(f, expon);
            dqdh = expon / pRange * factor / f;
        }

        // ... update total demand and its derivative

        q = qFull * factor;
        dqdh = qFull * dqdh;
    }
    return q;
}


//-----------------------------------------------------------------------------
///  Logistic Demand Model
//-----------------------------------------------------------------------------

LogisticDemandModel::LogisticDemandModel(double expon_) :
    DemandModel(expon_),
    a(0.0),
    b(0.0)
{}

double LogisticDemandModel::findDemand(Junction* junc, double p, double& dqdh)
{
    double f = 1.0;              // fraction of full demand
    double q = junc->fullDemand; // demand flow (cfs)
    double arg;                  // argument of exponential term
    double dfdh;                 // gradient of f w.r.t. pressure head

    // ... initialize derivative

    dqdh = 0.0;

    // ... check for positive demand and pressure range

    if ( junc->fullDemand > 0.0 && junc->pFull > junc->pMin )
    {
    	// ... find logistic function coeffs. a & b

    	setCoeffs(junc->pMin, junc->pFull);

        // ... prevent against numerical over/underflow

        arg = a + b*p;
        if (arg < -100.) arg = -100.0;
        else if (arg > 100.0) arg = 100.0;

        // ... find fraction of full demand (f) and its derivative (dfdh)

        f = exp(arg);
        f = f / (1.0 + f);
        f = max(0.0, min(1.0, f));
        dfdh = b * f * (1.0 - f);

        // ... evaluate demand and its derivative

        q = junc->fullDemand * f;
        dqdh = junc->fullDemand * dfdh;
    }
    return q;
}

void  LogisticDemandModel::setCoeffs(double pMin, double pFull)
{
    // ... computes logistic function coefficients
    //     assuming 99.9% of full demand at full pressure
    //     and 1% of full demand at minimum pressure.

    double pRange = pFull - pMin;
    a = (-4.595 * pFull - 6.907 * pMin) / pRange;
    b = 11.502 / pRange;
}

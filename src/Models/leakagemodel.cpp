/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "leakagemodel.h"
#include "Core/constants.h"

#include <cmath>
using namespace std;

const double C = 0.6 * sqrt(2*GRAVITY);

//-----------------------------------------------------------------------------
// Parent constructor and destructor
//-----------------------------------------------------------------------------

LeakageModel::LeakageModel() : lengthUcf(1.0), flowUcf(1.0)
{}

LeakageModel::~LeakageModel()
{}

//-----------------------------------------------------------------------------
// Leakage model factory
//-----------------------------------------------------------------------------

LeakageModel* LeakageModel::factory(const string model,
                                    const double ucfLength_,
                                    const double ucfFlow_)
{
    if      ( model == "POWER" ) return new PowerLeakageModel(ucfLength_, ucfFlow_);
    else if ( model == "FAVAD" ) return new FavadLeakageModel(ucfLength_);
    else                         return nullptr;
}


//-----------------------------------------------------------------------------
//  Power Leakage Model
//-----------------------------------------------------------------------------

PowerLeakageModel::PowerLeakageModel(const double ucfLength_, const double ucfFlow_)
{
    lengthUcf = ucfLength_;
    flowUcf = ucfFlow_;

    if ( lengthUcf == 1.0 ) pressureUcf = PSIperFT;
    else                    pressureUcf = MperFT;
}

double PowerLeakageModel::findFlow(double a, double b, double length, double h,
                                   double& dqdh)
{
    // no leakage for non-positive pressure head
    if ( h <= 0.0 )
    {
        dqdh = 0.0;
        return 0.0;
    }

    // ... find leakage using original units
    double q = a * pow(h * pressureUcf, b) * length * lengthUcf / 1000.0;

    // ... convert leakage to cfs
    q /= flowUcf;

    // ... compute half gradient in units of cfs/ft
    dqdh = b * q / h / 2.0;
    return q;
}


//-----------------------------------------------------------------------------
//  FAVAD Leakage Model
//-----------------------------------------------------------------------------

FavadLeakageModel::FavadLeakageModel(const double ucfLength_)
{
    lengthUcf = ucfLength_;
}

double FavadLeakageModel::findFlow(double a, double m, double length, double h,
                                   double& dqdh)
{
    // no leakage for non-positive pressure head
    if ( h <= 0.0 )
    {
        dqdh = 0.0;
        return 0.0;
    }

    // 'a' is leak area per 1000 pipe length units,
    // 'm' is change in 'a' per change in head (dimensionless),
    // convert 'a' to ft2 / 1000 ft
    a /= lengthUcf;

    // compute fixed & variable head portions of leakage in cfs
    // (C is orifice constant, 0.6 * sqrt(2g))
    double q1 = a * C * pow(h, 0.5) * length / 1000.0;
    double q2 = m * C * pow(h, 1.5) * length / 1000.0;

    // find half-gradient of leakage flow
    dqdh = (0.5 * q1 + 1.5 * q2) / h / 2.0;

    // return total leakage flow rate
    return q1 + q2;
}

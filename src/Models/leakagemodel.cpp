/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "leakagemodel.h"

#include <cmath>
using namespace std;

const double C = 0.6 * sqrt(2*32.2);

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
}

double PowerLeakageModel::findFlow(double a, double b, double length, double h, double& dqdh)
{
    // no leakage for non-positive pressure head
    if ( h <= 0.0 )
    {
        dqdh = 0.0;
        return 0.0;
    }

    // ... find flow in gpm/1000 ft (or lpm/km)
    double q = a * pow(h * lengthUcf, b);

    // ... convert flow to cfs
    q = q * (length * lengthUcf / 1000.0) / flowUcf;

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

double FavadLeakageModel::findFlow(double a, double m, double length, double h, double& dqdh)
{
    // no leakage for non-positive pressure head
    if ( h <= 0.0 )
    {
        dqdh = 0.0;
        return 0.0;
    }

    // ... convert area a from ft^2/1000ft (or m^2/km) to ft^2
    length = length * lengthUcf / 1000.0;
    a = a / lengthUcf / lengthUcf * length;

    // ... convert m to ft^2/ft
    m = m / lengthUcf * length;

    // ... compute fixed & variable head portions of leakage in cfs
    double q1 = a * C * pow(h, 0.5);
    double q2 = m * C * pow(h, 1.5);

    // ... find half-gradient of leakage flow
    dqdh = (0.5 * q1 + 1.5 * q2) / h / 2.0;

    // ... return total leakage flow rate
    return q1 + q2;
}

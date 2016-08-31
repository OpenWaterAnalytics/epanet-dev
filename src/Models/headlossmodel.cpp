/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

 // TO DO:
 // - replace polynomial used for D-W friction factor

#include "headlossmodel.h"
#include "Elements/pipe.h"
#include "Core/constants.h"

#include <cmath>
#include <algorithm>
using namespace std;

//-----------------------------------------------------------------------------

const double HW_EXP = 1.852;           // exponent for Hazen-Williams formula

////////////////////////////////////////////////////////////
//// Replace with more recently derived approximation.  ////
////////////////////////////////////////////////////////////
// Constants used for computing Darcy-Weisbach friction factor
const double A1 = 0.314159265359e04;   // 1000*PI
const double A2 = 0.157079632679e04;   // 500*PI
const double A3 = 0.502654824574e02;   // 16*PI
const double A4 = 6.283185307;         // 2*PI
const double A8 = 4.61841319859;       // 5.74*(PI/4)^.9
const double A9 = -8.685889638e-01;    // -2/ln(10)
const double AA = -1.5634601348;       // -2*.9*2/ln(10)
const double AB = 3.28895476345e-03;   // 5.74/(4000^.9)
const double AC = -5.14214965799e-03;  // AA*AB

//-----------------------------------------------------------------------------

// Darcy-Weisbach friction factor
double frictionFactor(double q, double e, double s, double& dfdq);

//-----------------------------------------------------------------------------

// Parent constructor

HeadLossModel::HeadLossModel(double viscos) :
    viscosity(viscos)
{}

// Parent destructor

HeadLossModel::~HeadLossModel() {}

//-----------------------------------------------------------------------------
// Head loss model factory
//-----------------------------------------------------------------------------

HeadLossModel* HeadLossModel::factory(const string model, double viscos)
{
    if ( model == "H-W" ) return new HW_HeadLossModel(viscos);
    if ( model == "D-W" ) return new DW_HeadLossModel(viscos);
    if ( model == "C-M" ) return new CM_HeadLossModel(viscos);
    return nullptr;
}

// Head loss for a closed link

void HeadLossModel::findClosedHeadLoss(double flow,
                                       double& headLoss,
                                       double& gradient)
{
    gradient = HIGH_RESISTANCE;
    headLoss = HIGH_RESISTANCE * flow;
}

// Head loss addition for link with a check valve

void HeadLossModel::addCVHeadLoss(double q, double& headLoss, double& gradient)
{
    double a = HIGH_RESISTANCE * q;
    double b = sqrt(a*a + HEAD_EPSILON);
    headLoss += (a - b) / 2.0;
    gradient += HIGH_RESISTANCE * ( 1.0 - a/b) / 2.0;
}

//-----------------------------------------------------------------------------
//  Hazen-Williams Head Loss Model
//-----------------------------------------------------------------------------

HW_HeadLossModel::HW_HeadLossModel(double viscos) : HeadLossModel(viscos)
{}

void HW_HeadLossModel::setResistance(Pipe* pipe)
{
    pipe->resistance =
        4.727 * pipe->length / pow(pipe->roughness, HW_EXP) /
        pow(pipe->diameter, 4.871);
    pipe->resistance = min(pipe->resistance, HIGH_RESISTANCE);
}

void HW_HeadLossModel::findHeadLoss(Pipe* pipe, double flow, double& headLoss,
    double& gradient)
{
    double q = abs(flow);
    double r = pipe->resistance;
    double k = pipe->lossFactor;

    gradient = HW_EXP * r * pow(q, HW_EXP-1.0);
    if ( gradient < MIN_GRADIENT )
    {
        gradient = MIN_GRADIENT;
        headLoss = q * gradient;
    }
    else headLoss = q * gradient / HW_EXP;
    if (k > 0.0)
    {
        headLoss += k * q * q;
        gradient += 2.0 * k * q;
   	}

    // ... give proper sign to head loss

    if (flow < 0.0) headLoss = -headLoss;
}


//-----------------------------------------------------------------------------
//  Chezy-Manning Head Loss Model
//-----------------------------------------------------------------------------

CM_HeadLossModel::CM_HeadLossModel(double viscos) : HeadLossModel(viscos)
{}

void CM_HeadLossModel::setResistance(Pipe* pipe)
{
    double d = pipe->diameter;
    double r = 4.0 * pipe->roughness / (1.49 * PI * d * d);
    pipe->resistance = r * r * pow(d/4.0, -1.333) * pipe->length;
    pipe->resistance = min(pipe->resistance, HIGH_RESISTANCE);
}

void CM_HeadLossModel::findHeadLoss(
        Pipe* pipe, double flow, double& headLoss, double& gradient)
{
    double q = abs(flow);
    double r = pipe->resistance;
    double k = pipe->lossFactor;

    gradient = 2.0 * r * q;
    if ( gradient < MIN_GRADIENT )
    {
        gradient = MIN_GRADIENT;
        headLoss = q * gradient;
    }
    else headLoss = q * gradient / 2.0;
    if (k > 0.0)
    {
        headLoss += k * q * q;
        gradient += 2.0 * k * q;
   	}
}


//-----------------------------------------------------------------------------
//  Darcy-Weisbach Head Loss Model
//-----------------------------------------------------------------------------

DW_HeadLossModel::DW_HeadLossModel(double viscos) : HeadLossModel(viscos)
{}

void DW_HeadLossModel::setResistance(Pipe* pipe)
{
    double d = pipe->diameter;
    double a = PI * d * d / 4.0;

    // ... D-W formula is resistance * friction factor * flow^2

    pipe->resistance = pipe->length / 2.0 / GRAVITY / d / a / a;
    pipe->resistance = min(pipe->resistance, HIGH_RESISTANCE);
}

void DW_HeadLossModel::findHeadLoss(Pipe* pipe, double flow,  double& headLoss,
    double& gradient)
{
    double q = abs(flow);
    double r = pipe->resistance;
    double k = pipe->lossFactor;
    double s = viscosity * pipe->diameter;

    // ... use Hagen-Poiseuille formula for laminar flow (Re <= 2000)

    if (q <= A2 * s)
    {
        r = 16.0 * PI * s * pipe->resistance;
        headLoss = flow * (r + k * q);
        gradient = r + 2.0 * k * q;
    }

    // ... use Colebrook formula for turbulent flow

    else
    {
        double dfdq = 0.0;
        double e = pipe->roughness / pipe->diameter;
        double f = frictionFactor(q, e, s, dfdq);
        double r1 = f * r + k;
        headLoss = r1 * q * flow;
        gradient = (2.0 * r1 * q) + (dfdq * r * q * q);
    }
}

double frictionFactor(double q, double e, double s, double& dfdq)
//
//   Purpose: computes Darcy-Weisbach friction factor
//   Input:   q = flow rate (cfs)
//            e = roughness / diameter
//            s = viscosity * diameter
//   Output:  returns Darcy-Weisbach friction factor and its derivative dfdq
//
////////////////////////////////////////////////////////////
//// Replace with more recently derived approximation.  ////
////////////////////////////////////////////////////////////
//   Uses interpolating polynomials developed by
//   E. Dunlop for transition flow from 2000 < Re < 4000.
//
{
    double f;                // friction factor
    double x1, x2, x3, x4,
           y1, y2, y3,
           fa, fb, r;
    double w = q / s;        // Re*Pi/4

    // for Re >= 4000 use Colebrook Formula

    if ( w >= A1 )
    {
        y1 = A8 / pow(w, 0.9);
        y2 = e / 3.7 + y1;
        y3 = A9 * log(y2);
        f = 1.0 / (y3*y3);
        dfdq = 1.8 * f * y1 * A9 / y2 / y3 / q;
    }

    // otherwise use interpolation formula

    else
    {
        y2 = e / 3.7 + AB;
        y3 = A9 * log(y2);
        fa = 1.0 / (y3*y3);
        fb = (2.0 + AC / (y2*y3)) * fa;
        r = w / A2;
        x1 = 7.0 * fa - fb;
        x2 = 0.128 - 17.0 * fa + 2.5 * fb;
        x3 = -0.128 + 13.0 * fa - (fb + fb);
        x4 = r * (0.032 - 3.0 * fa + 0.5 *fb);
        f = x1 + r * (x2 + r * (x3 + x4));
        dfdq = (x2 + 2.0 * r * (x3 + x4)) / s / A2;
    }
    return f;
}

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "pumpcurve.h"
#include "curve.h"
#include "Core/network.h"
#include "Core/error.h"

#include <cmath>
#include <algorithm>
using namespace std;

//-----------------------------------------------------------------------------

const double BIG_NUMBER = 1.0e10;      //!< numerical infinity
const double TINY_NUMBER = 1.0e-6;     //!< numerical zero

//-----------------------------------------------------------------------------

//  Constructor

PumpCurve::PumpCurve() :
    curveType(NO_CURVE),
    curve(nullptr),
    horsepower(0.0),
    qInit(0.0),
    qMax(0.0),
    hMax(0.0),
    h0(0.0),
    r(0.0),
    n(0.0),
    qUcf(1.0),
    hUcf(1.0)
{
}

//-----------------------------------------------------------------------------

//  Destructor

PumpCurve::~PumpCurve() {}

//-----------------------------------------------------------------------------

//  Assigns head v. flow data (from a Curve object) to a pump curve.

void PumpCurve::setCurve(Curve* c)
{
    curve = c;
}

//-----------------------------------------------------------------------------

//  Extracts a pump curve's parameters from its data points.

int PumpCurve::setupCurve(Network* network)
{
    int err = 0;

    // ... assign unit conversion factors

    qUcf = network->ucf(Units::FLOW);
    hUcf = network->ucf(Units::LENGTH);

    // ... constant HP pump

    if (horsepower > 0.0 && curve == 0)
    {
        setupConstHpCurve();
    }

    // ... a pump curve was supplied

    else if (curve != 0) {

        // ... power curve supplied
        if (curve->size() == 1 ||
            (curve->size() == 3 && curve->x(0) == 0.0) )
        {
            err = setupPowerFuncCurve();
        }

        // ... custom curve supplied
        else err = setupCustomCurve();
    }

    // ... error - no curve was supplied

    else err = NetworkError::NO_PUMP_CURVE;
    qInit /= qUcf;
    return err;
}

//-----------------------------------------------------------------------------

//  Finds the head loss and its gradient for a pump curve.

void PumpCurve::findHeadLoss(
        double speed, double flow, double& headLoss, double& gradient)
{
    double q = abs(flow);
    switch (curveType)
    {
    case CUSTOM:
        customCurveHeadLoss(speed, q, headLoss, gradient); break;

    case CONST_HP:
        constHpHeadLoss(speed, q, headLoss, gradient); break;

    case POWER_FUNC:
        powerFuncHeadLoss(speed, q, headLoss, gradient); break;
    }
}

//-----------------------------------------------------------------------------

//  Sets the pump curve parameters for a constant horsepower curve.

void PumpCurve::setupConstHpCurve()
{
    curveType = CONST_HP;

    // ... Pump curve coefficients (head = h0 + r*flow^n)

    h0 = 0.0;
    r  = -8.814 * horsepower;
    n  = -1.0;

    // ... unit conversion factors

    qUcf = 1.0;
    hUcf = 1.0;

    // ... pump curve limits

    hMax  = BIG_NUMBER;         // No head limit
    qMax  = BIG_NUMBER;         // No flow limit
    qInit = 1.0;                // Init. flow = 1 cfs
}

//-----------------------------------------------------------------------------

//  Sets pump curve parameters for a power function curve.

int PumpCurve::setupPowerFuncCurve()
{
    // ... declare control points

    double h1, h2, q1, q2;

    // ... 1-point pump curve - fill in shutoff head & max. flow

    if (curve->size() == 1)
    {
        curveType = POWER_FUNC;
        q1 = curve->x(0);
        h1 = curve->y(0);
        h0 = 1.33334 * h1;
        q2 = 2.0 * q1;
        h2 = 0.0;
    }

    // ... 3-point pump curve with shutoff head

    else if (curve->size() == 3 && curve->x(0) == 0.0)
    {
        curveType = POWER_FUNC;
        h0 = curve->y(0);
        q1 = curve->x(1);
        h1 = curve->y(1);
        q2 = curve->x(2);
        h2 = curve->y(2);
    }

    else return NetworkError::INVALID_PUMP_CURVE;

    // ... check for valid control points

    if (  h0      < TINY_NUMBER ||
          h0 - h1 < TINY_NUMBER ||
          h1 - h2 < TINY_NUMBER ||
          q1      < TINY_NUMBER ||
          q2 - q1 < TINY_NUMBER
          ) return NetworkError::INVALID_PUMP_CURVE;

    // ... find curve coeffs. from control points

    double h4 = h0 - h1;
    double h5 = h0 - h2;
    n = log(h5/h4) / log(q2/q1);
    if (n <= 0.0 || n > 20.0) return NetworkError::INVALID_PUMP_CURVE;
    r = -h4 / pow(q1, n);
    if (r >= 0.0) return NetworkError::INVALID_PUMP_CURVE;

    // ... assign pump curve limits

    hMax = h0;
    qMax = pow(-h0/r, 1.0/n);
    qInit = q1;
    return 0;
}

//-----------------------------------------------------------------------------

//  Sets pump curve parameters for a custom pump curve.

int PumpCurve::setupCustomCurve()
{
    // ... check that head (y) decreases with increasing flow (x)

    for (int m = 1; m < curve->size(); m++)
    {
        if (curve->y(m-1) - curve->y(m) < TINY_NUMBER || curve->y(m) < 0.0)
        {
            return NetworkError::INVALID_PUMP_CURVE;
        }
    }

    // ... extrapolate to zero flow to find shutoff head

    double slope = (curve->y(0) - curve->y(1)) /
                   (curve->x(1) - curve->x(0));
    hMax = curve->y(0) + slope * curve->x(0);

    // ... extrapolate to zero head to find max. flow

    int k = curve->size() - 1;
    slope = (curve->x(k) - curve->x(k-1)) /
            (curve->y(k-1) - curve->y(k));
    qMax  = curve->x(k) + slope * curve->y(k);

    // ... curve exponent is 1 (curve is piece-wise linear)

    n = 1.0;

    // ... initial flow is curve mid-point

    qInit = (curve->x(0) + curve->x(k)) / 2.0;
    curveType = CUSTOM;
    return 0;
}

//-----------------------------------------------------------------------------

//  Finds head loss and its gradient for a custom pump curve.

void PumpCurve::customCurveHeadLoss(
        double speed, double flow, double& headLoss, double& gradient)
{
    // ... convert flow value to pump curve units

    double q = abs(flow) * qUcf;

    // ... find slope and intercept of curve segment

    curve->findSegment(q, r, h0);

    // ... adjust slope and intercept for pump speed

    h0 = h0 * speed * speed;
    r = r * speed;

    // ... evaluate head loss (negative of pump head) and its gradient

    headLoss = -(h0 + r*q);
    gradient = -r;

    // ... convert results to internal units

    headLoss /= hUcf;
    gradient *= qUcf / hUcf;
}

//-----------------------------------------------------------------------------

//  Finds head loss and its gradient for a constant horsepower pump curve.

void PumpCurve::constHpHeadLoss(
        double speed, double flow, double& headLoss, double& gradient)
{
    double w = speed * speed * r;
    double q = max(flow, 1.0e-6);
    headLoss = w / q;
    gradient = abs(headLoss / q);
}

//-----------------------------------------------------------------------------

//  Finds head loss and its gradient for a power function pump curve.

void PumpCurve::powerFuncHeadLoss(
        double speed, double flow, double& headLoss, double& gradient)
{
    // ... convert flow to pump curve units

    double q = abs(flow) * qUcf;

    // ... adjust curve coeffs. for pump speed

    double h01 = h0;
    double r1 = r;
    double w = 1.0;
    if (speed != 1.0)
    {
        w = speed * speed;
        h01 *= w;
        w = w / pow(speed, n);
    }

    // ... evaluate head loss (negative of pump head) and its gradient

    r1 = w * r * pow(q, n);
    headLoss = -(h01 + r1);
    gradient = -(n * r1 / q);

    // ... convert results to internal units

    headLoss /= hUcf;
    gradient *= qUcf / hUcf;
}

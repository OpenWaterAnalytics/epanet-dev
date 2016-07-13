/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "curve.h"
#include "Utilities/utilities.h"

#include <iomanip>
using namespace std;

//-----------------------------------------------------------------------------

const char* Curve::CurveTypeWords[] =
    {"", "PUMP", "EFFICIENCY", "VOLUME", "HEADLOSS", 0};

//-----------------------------------------------------------------------------

Curve::Curve(string name_) :
    Element(name_),
    type(UNKNOWN)
{}

Curve::~Curve()
{
    xData.clear();
    yData.clear();
}

//-----------------------------------------------------------------------------

void Curve::findSegment(double xseg, double& slope, double& intercept)
{
    int n = xData.size();
    int segment = n-1;

    if (n == 1)
    {
        intercept = 0.0;
        if (xData[0] == 0.0) slope = 0.0;
        else slope = yData[0] / xData[0];
    }

    else
    {
        for (int i = 1; i < n; i++)
        {
            if (xseg <= xData[i])
            {
                segment = i;
                break;
            }
        }
        slope = (yData[segment] - yData[segment-1]) /
                (xData[segment] - xData[segment-1]);
        intercept = yData[segment] - slope * xData[segment];
    }
}

//-----------------------------------------------------------------------------

double Curve::getYofX(double x)
{
    if ( x <= xData[0] ) return yData[0];

    for (vector<double>::size_type i = 1; i < xData.size(); i++)
    {
        if ( x <= xData[i] )
        {
            double dx = xData[i] - xData[i-1];
            if ( dx == 0.0 ) return yData[i-1];
            return yData[i-1] + (x - yData[i-1]) / dx * (yData[i] - yData[i-1]);
        }
    }
    return yData[xData.size()-1];
}

//-----------------------------------------------------------------------------

double Curve::getXofY(double y)
// Assumes Y is increasing with X
{
    if ( y <= yData[0] ) return xData[0];

    for (vector<double>::size_type i = 1; i < yData.size(); i++)
    {
    	if ( y <= yData[i] )
    	{
            double dy = yData[i] - yData[i-1];
            if ( dy == 0.0 ) return xData[i-1];
            return xData[i-1] + (y - yData[i-1]) / dy * (xData[i] - xData[i-1]);
        }
    }
    return xData[yData.size()-1];
}

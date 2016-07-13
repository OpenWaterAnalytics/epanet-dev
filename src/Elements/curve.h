/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file curve.h
//! \brief Describes the Curve class.

#ifndef CURVE_H_
#define CURVE_H_

#include "Elements/element.h"

#include <string>
#include <iostream>
#include <vector>

//! \class Curve
//! \brief An ordered collection of x,y data pairs.
//!
//! Curves can be used to describe how tank volume varies with height, how
//! pump head or efficiency varies with flow, or how a valve's head loss
//! varies with flow.

//  NOTE: Curve data are stored in the user's original units.
//-----------------------------------------------------------------------------

class Curve: public Element
{
  public:

    // Curve type enumeration
    enum CurveType   {UNKNOWN, PUMP, EFFICIENCY, VOLUME, HEADLOSS};

    // Names of curve types
    static const char* CurveTypeWords[];

    // Constructor/Destructor
    Curve(std::string name_);
    ~Curve();

    // Data provider methods
    void   setType(int curveType);
    void   addData(double x, double y);

    // Data retrieval methods
    int    size();
    int    curveType();
    double x(int index);
    double y(int index);
    void   findSegment(double xseg, double& slope, double& intercept);
    double getYofX(double x);
    double getXofY(double y);

  private:
    CurveType               type;           //!< curve type
    std::vector<double>     xData;          //!< x-values
    std::vector<double>     yData;          //!< y-values
};

//-----------------------------------------------------------------------------
//    Inline Functions
//-----------------------------------------------------------------------------
inline  void   Curve::setType(int curveType)
               { type = (CurveType)curveType; }

inline  void   Curve::addData(double x, double y)
               { xData.push_back(x); yData.push_back(y);}

inline  int    Curve::size() { return xData.size(); }

inline  int    Curve::curveType() { return (int)type; }

inline  double Curve::x(int i) { return xData.at(i); }

inline  double Curve::y(int i) { return yData.at(i); }

#endif

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file curveparser.h
//! \brief Describes the CurveParser class.

#ifndef CURVEPARSER_H_
#define CURVEPARSER_H_

#include <string>
#include <vector>

class Curve;

//! \class CurveParser
//! \brief The CurveParser class is used to parse a line of input data for
//!        curve data (pairs of x,y values).

class CurveParser
{
public:
    CurveParser() {}
    ~CurveParser() {}
    void parseCurveData(Curve* curve, std::vector<std::string>& tokens);
};

#endif

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "curveparser.h"
#include "Elements/curve.h"
#include "Core/error.h"
#include "Utilities/utilities.h"

using namespace std;

//-----------------------------------------------------------------------------

void CurveParser::parseCurveData(Curve* curve, vector<string>& tokenList)
{
    // Formats are:
    //   curveName curveType
    //   curveName x1 y1 x2 y2 ...

    // ... check for enough tokens

    int nTokens = tokenList.size();
    if ( nTokens < 2 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    // ... check if second token is curve type keyword

    int curveType = Utilities::findMatch(tokens[1], Curve::CurveTypeWords);
    if (curveType > 0)
    {
        curve->setType(curveType);
        return;
    }

    // ... otherwise read in pairs of x,y values

    double xx;
    double yy;
    int i = 1;
    while ( i < nTokens )
    {
        if ( !Utilities::parseNumber(tokens[i], xx) )
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[i]);
        }
        i++;
        if ( i >= nTokens ) throw InputError(InputError::TOO_FEW_ITEMS, "");
        if ( !Utilities::parseNumber(tokens[i], yy) )
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[i]);
        }
        curve->addData(xx, yy);
        i++;
    }
}

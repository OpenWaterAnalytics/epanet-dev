/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "patternparser.h"
#include "Elements/pattern.h"
#include "Core/error.h"
#include "Utilities/utilities.h"

using namespace std;

//-----------------------------------------------------------------------------

void PatternParser::parsePatternData(Pattern* pattern, vector<string>& tokens)
{
    if ( pattern->type == Pattern::FIXED_PATTERN )
    {
        parseFixedPattern(pattern, tokens);
    }
    else parseVariablePattern(pattern, tokens);
}

//-----------------------------------------------------------------------------

void PatternParser::parseFixedPattern(Pattern* pattern, vector<string>& tokenList)
{
    // Formats are:
    //     PatternName  FIXED  (interval)
    //     PatternName  factor1  factor2  ...

    // ... check for enough tokens

    int nTokens = tokenList.size();
    if ( nTokens < 2 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    // ... cast pattern to FixedPattern

    FixedPattern* fixedPat = static_cast<FixedPattern*>(pattern);

    // ... check if next token is the pattern type keyword

    if ( Utilities::match(tokens[1], "FIXED") )
    {
        // check if next token is the pattern interval

        if ( nTokens > 2 )
        {
            string s2 = "";
            int interval = Utilities::getSeconds(tokens[2], s2);
            if ( interval <= 0 ) throw InputError(InputError::INVALID_TIME, tokens[2]);
            fixedPat->setTimeInterval(interval);

        }
        return;
    }

    // ... read in pattern factors

    double factor;
    int i = 1;
    while ( i < nTokens )
    {
        if ( !Utilities::parseNumber(tokens[i], factor) )
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[i]);
        }
        fixedPat->addFactor(factor);
        i++;
    }
}

//-----------------------------------------------------------------------------

void PatternParser::parseVariablePattern(Pattern* pattern, vector<string>& tokenList)
{
    // Formats are:
    //     PatternName  VARIABLE
    //     PatternName  time1  factor1  time2  factor2  ...

    // ... check for enough tokens

    int nTokens = tokenList.size();
    if ( nTokens < 2 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    // ... cast pattern to VariablePattern

    VariablePattern* varPat = static_cast<VariablePattern*>(pattern);

    // ... return if second token is the pattern type keyword

    if ( Utilities::match(tokens[1], "VARIABLE") ) return;

    // ... read in pairs of times and pattern factors
    //     (times can be in decimal hours or hours:minutes format)

    string timeUnits("");
    int    seconds;
    double factor;
    int i = 1;
    while ( i < nTokens )
    {
        seconds = Utilities::getSeconds(tokens[i], timeUnits);
        if ( seconds < 0 )
        {
            throw InputError(InputError::INVALID_TIME, tokens[i] + " " + timeUnits);
        }
        i++;
        if ( i >= nTokens ) throw InputError(InputError::TOO_FEW_ITEMS, "");
        if ( !Utilities::parseNumber(tokens[i], factor) )
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[i]);
        }
        varPat->addTime(seconds);
        varPat->addFactor(factor);
        i++;
    }
}

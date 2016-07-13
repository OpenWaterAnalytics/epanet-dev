/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file patternparser.h
//! \brief Description of the PatternParser class.

#ifndef PATTERNPARSER_H_
#define PATTERNPARSER_H_

#include <string>
#include <vector>

class Network;
class Pattern;

class PatternParser
{
  public:
    PatternParser() {}
    ~PatternParser() {}
    void parsePatternData(Pattern* pattern, std::vector<std::string>& tokens);

  protected:
    void parseFixedPattern(Pattern* pattern, std::vector<std::string>& tokens);
    void parseVariablePattern(Pattern* pattern, std::vector<std::string>& tokens);
};

#endif

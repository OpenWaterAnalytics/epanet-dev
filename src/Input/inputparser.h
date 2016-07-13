/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file inputparser.h
//! \brief Describes the InputParser class.

#ifndef INPUTPARSER_H_
#define INPUTPARSER_H_

#include "Input/nodeparser.h"
#include "Input/linkparser.h"
#include "Input/patternparser.h"
#include "Input/curveparser.h"
#include "Input/optionparser.h"
#include "Input/controlparser.h"

#include <string>
#include <vector>

class Network;

//! \class InputParser
//! \brief Parses lines of network input data from a text file.
//!
//! This is an abstract class with two derived child classes:
//! - ObjectParser identifies each new element of a network as the
//!   InputReader makes a first pass through the network input file;
//! - PropertyParser reads the properties of these elements as the
//!   InputReader makes a second pass through the input file.

class InputParser
{
  public:
    InputParser(Network* nw);
    virtual ~InputParser();
    virtual void parseLine(std::string& line, int section) = 0;
    Network* network;
};

//! \class ObjectParser
//! \brief Parses a new network element from a line of input.
//!
//! Once a new element (junction node, pipe link, time pattern, etc) is
//! detected in a line of input, a new object is created with the ID name
//! parsed from the input line.

class ObjectParser : public InputParser
{
  public:
    ObjectParser(Network* nw);
    void parseLine(std::string& line, int section);
};

//! \class PropertyParser
//! \brief Parses a network element's properties from a line of input.

class PropertyParser : public InputParser
{
  public:
    PropertyParser(Network* nw);
    void parseLine(std::string& line, int section);

  private:
    NodeParser     nodeParser;
    LinkParser     linkParser;
    PatternParser  patternParser;
    CurveParser    curveParser;
    OptionParser   optionParser;
    ControlParser  controlParser;
    std::vector<std::string> tokens;

    void parseNodeProperty(int type, std::string& nodeName);
    void parseLinkProperty(int type, std::string& linkName);
    void parseReactProperty(std::string& reactType);
};

#endif

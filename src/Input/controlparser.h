/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file controlparser.h
//! \brief Describes the ControlParser class.

#ifndef CONTROLPARSER_H_
#define CONTROLPARSER_H_

#include <sstream>
#include <string>

class Node;
class Link;
class Network;

//! \class ControlParser
//! \brief Parses a control statement from a line of text.
//!
//! The ControlParser class is used to parse a line of a simple control
//! statement read from a text file.

class ControlParser
{
  public:
    ControlParser();
    ~ControlParser() {}
    void parseControlLine(std::string& line, Network* network);

  private:
    void   initializeControlSettings();
    void   parseLinkSetting(Network* network);
    void   parseLevelSetting(Network* network);
    void   parseTimeSetting();
    void   parseTimeOfDaySetting();
    void   createControl(Network* network);

    std::istringstream  sin;
    int                 controlType;
    Link*               link;
    int                 linkStatus;
    double              linkSetting;
    Node*               node;
    int                 levelType;
    double              nodeSetting;
    int                 timeSetting;
};

#endif

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file linkparser.h
//! \brief Describes the NodeParser class.

#ifndef LINKPARSER_H_
#define LINKPARSER_H_

#include <string>
#include <vector>

class Link;
class Network;

//! \class LinkParser
//! \brief The LinkParser class is used to parse lines of input data
//!        for network links read from a text file.

class LinkParser
{
  public:
    LinkParser() {}
    ~LinkParser() {}

    void parseLinkData(std::string& id, Network* nw, std::vector<std::string>& tokens);
    void parseStatus(Link* link, std::vector<std::string>& tokens);
    void parseLeakage(Link* link, std::vector<std::string>& tokens);
    void parseEnergy(Link* link, Network* network, std::vector<std::string>& tokens);
    void parseReaction(Link* link, int type, std::vector<std::string>& tokens);
};

#endif

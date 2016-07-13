/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file nodeparser.h
//! \brief Description of the NodeParser class.

#ifndef NODEPARSER_H_
#define NODEPARSER_H_

#include <sstream>
#include <string>
#include <vector>

class Node;
class Network;

//! \class NodeParser
//! \brief The NodeParser class is used to parse lines of input data
//!        for network nodes read from a text file.

class NodeParser
{
  public:
    NodeParser() {}
    ~NodeParser() {}

    void parseNodeData(std::string& nodeName, Network* nw, std::vector<std::string>& tokens);
    void parseDemand(Node* node, Network* nw, std::vector<std::string>& tokens);
    void parseEmitter(Node* node, Network* nw, std::vector<std::string>& tokens);
    void parseCoords(Node* node, std::vector<std::string>& tokens);
    void parseInitQual(Node* node, std::vector<std::string>& tokens);
    void parseQualSource(Node* node, Network* nw, std::vector<std::string>& tokens);
    void parseTankMixing(Node* node, std::vector<std::string>& tokens);
    void parseTankReact(Node* node, std::vector<std::string>& tokens);
};

#endif


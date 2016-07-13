/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

 // TO DO:
 // - add support for Rule-Based controls

#include "inputparser.h"
#include "inputreader.h"
#include "Core/network.h"
#include "Core/error.h"
#include "Elements/node.h"
#include "Elements/link.h"
#include "Elements/pattern.h"
#include "Utilities/utilities.h"

#include <sstream>
using namespace std;

//-----------------------------------------------------------------------------
//  Input File Keywords
//-----------------------------------------------------------------------------
static const char* w_Pump = "PUMP";
//static const char* w_Rule = "RULE";
static const char* w_Variable = "VARIABLE";
static const char* w_Bulk = "BULK";
static const char* w_Wall = "WALL";
static const char* w_Tank = "TANK";

//-----------------------------------------------------------------------------

// Link properties enumeration
enum LinkParam {STATUS, LEAKAGE, ENERGY, BULK, WALL};

//-----------------------------------------------------------------------------

//  InputParser constructor and destructor

InputParser::InputParser(Network* nw) : network(nw)
{
}

InputParser::~InputParser()
{
}

//-----------------------------------------------------------------------------

//  ObjectParser constructor

ObjectParser::ObjectParser(Network* nw) : InputParser(nw)
{
}

//  Parse a line from input file to create a new object.

void ObjectParser::parseLine(string& line, int section)
{
    string s1;
    string s2;
    int    type = 0;

    // ... read first string token from input line

    if ( network == 0 ) return;
    istringstream iss(line);
    iss >> s1;

    // ... create new object whose type is determined by current file section

    switch(section)
    {
    case InputReader::JUNCTION:
        if ( network->indexOf(Element::NODE, s1) >= 0 )
        {
            throw InputError(InputError::DUPLICATE_ID, s1);
        }
        if ( !network->addElement(Element::NODE, Node::JUNCTION, s1) )
        {
            throw InputError(InputError::CANNOT_CREATE_OBJECT, s1);
        }
        break;

    case InputReader::RESERVOIR:
        if ( network->indexOf(Element::NODE, s1) >= 0 )
        {
            throw InputError(InputError::DUPLICATE_ID, s1);
        }
        if ( !network->addElement(Element::NODE, Node::RESERVOIR, s1) )
        {
            throw InputError(InputError::CANNOT_CREATE_OBJECT, s1);
        }
        break;

    case InputReader::TANK:
        if ( network->indexOf(Element::NODE, s1) >= 0 )
        {
            throw InputError(InputError::DUPLICATE_ID, s1);
        }
        if ( !network->addElement(Element::NODE, Node::TANK, s1) )
        {
            throw InputError(InputError::CANNOT_CREATE_OBJECT, s1);
        }
        break;

    case InputReader::PIPE:
        if ( network->indexOf(Element::LINK, s1) >= 0 )
        {
            throw InputError(InputError::DUPLICATE_ID, s1);
        }
        if ( !network->addElement(Element::LINK, Link::PIPE, s1) )
        {
            throw InputError(InputError::CANNOT_CREATE_OBJECT, s1);
        }
        break;

    case InputReader::PUMP:
        if ( network->indexOf(Element::LINK, s1) >= 0 )
        {
            throw InputError(InputError::DUPLICATE_ID, s1);
        }
        if ( !network->addElement(Element::LINK, Link::PUMP, s1) )
        {
            throw InputError(InputError::CANNOT_CREATE_OBJECT, s1);
        }
        break;

    case InputReader::VALVE:
        if ( network->indexOf(Element::LINK, s1) >= 0 )
        {
            throw InputError(InputError::DUPLICATE_ID, s1);
        }
        if ( !network->addElement(Element::LINK, Link::VALVE, s1) )
        {
            throw InputError(InputError::CANNOT_CREATE_OBJECT, s1);
        }
        break;

    case InputReader::PATTERN:
        // Check if pattern already created
        if ( network->indexOf(Element::PATTERN, s1 ) >= 0) break;

        // Check if pattern is Fixed or Variable
        iss >> s2;
        type = Pattern::FIXED_PATTERN;
        if (Utilities::match(s2, w_Variable)) type = Pattern::VARIABLE_PATTERN;

        if ( !network->addElement(Element::PATTERN, type, s1) )
        {
            throw InputError(InputError::CANNOT_CREATE_OBJECT, s1);
        }
        break;

    case InputReader::CURVE:
        // Check if curve already created
        if ( network->indexOf(Element::CURVE, s1) >= 0 ) break;
        if ( !network->addElement(Element::CURVE, 0, s1) )
        {
            throw InputError(InputError::CANNOT_CREATE_OBJECT, s1);
        }
        break;

/*
    case InputReader::RULE:
        // Check for Rule keyword
        if (Utilities::match(s1, w_Rule))
        {
            // Parse rule's name
            sin >> s2;
            if (sin.fail()) return InputError::TOO_FEW_ITEMS;

            // Check if rule with same name already exists
            if (network->indexOf(Element::CONTROL, s2) >= 0)
                return InputError::DUPLICATE_ID;

            // Add new rule
            network->addElement(Element::CONTROL, s2);
        }
        break;
*/
    }
}

//-----------------------------------------------------------------------------

//  PropertyParser Constructor

PropertyParser::PropertyParser(Network* nw) : InputParser(nw)
{
}

//-----------------------------------------------------------------------------

//  Parse object properties from a line read from the input file.

void PropertyParser::parseLine(string& line, int section)
{
    if ( network == 0 ) return;

    // ... for [TITLE] section, add line to network's title/notes

    if ( section == InputReader::TITLE )
    {
        network->addTitleLine(line);
        return;
    }

    // ... for Control, apply special control parser

    if ( section == InputReader::CONTROL )
    {
	    controlParser.parseControlLine(line, network);
	    return;
    }

    // ... split the input line into an array of string tokens

    tokens.clear();
    Utilities::split(tokens, line);
    string id = tokens[0];

    // ... use appropriate parsing function for current input section

    switch(section)
    {

        // Nodes
        case InputReader::JUNCTION:
        case InputReader::RESERVOIR:
        case InputReader::TANK:
            nodeParser.parseNodeData(id, network, tokens);
            break;

        // Links
        case InputReader::PIPE:
        case InputReader::PUMP:
        case InputReader::VALVE:
            linkParser.parseLinkData(id, network, tokens);
            break;

        // Patterns
        case InputReader::PATTERN:
            if ( !network->pattern(id) ) throw InputError(InputError::UNDEFINED_OBJECT, id);
	        patternParser.parsePatternData(network->pattern(id), tokens);
	        break;

        // Curves
        case InputReader::CURVE:
            if ( !network->curve(id) ) throw InputError(InputError::UNDEFINED_OBJECT, id);
            curveParser.parseCurveData(network->curve(id), tokens);
	        break;

        // Rules
        case InputReader::RULE:
            break;

        // Node properties
        case InputReader::EMITTER:
        case InputReader::DEMAND:
        case InputReader::COORD:
        case InputReader::QUALITY:
        case InputReader::SOURCE:
        case InputReader::MIXING:
	        parseNodeProperty(section, id);
	        break;

        // Link properties
        case InputReader::STATUS:  parseLinkProperty(STATUS, id);  break;
        case InputReader::LEAKAGE: parseLinkProperty(LEAKAGE, id); break;

        // Energy usage parameters
        case InputReader::ENERGY:
            if ( Utilities::match(id, w_Pump) )
            {
                if ( tokens.size() < 2 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
                id = tokens[1];
                parseLinkProperty(ENERGY, id);
            }
            else optionParser.parseEnergyOption(network, tokens);
            break;

        // General Options
        case InputReader::OPTION:
            optionParser.parseOption(network, tokens);
            break;

        // Time Options
        case InputReader::TIME:
            optionParser.parseTimeOption(network, tokens);
            break;

        // Water quality reaction parameters
        case InputReader::REACTION:
            parseReactProperty(id);
            break;

        // Reporting options
        case InputReader::REPORT:
            optionParser.parseReportOption(network, tokens);
            break;
    }
}

//-----------------------------------------------------------------------------

//  Read a node property from an input stream

void PropertyParser::parseNodeProperty(int type, string& nodeName)
{
    Node* node = network->node(nodeName);
    if ( node == nullptr ) throw InputError(InputError::UNDEFINED_OBJECT, nodeName);
    switch (type)
    {
        case InputReader::EMITTER:
            nodeParser.parseEmitter(node, network, tokens);
            break;
        case InputReader::DEMAND:
            nodeParser.parseDemand(node, network, tokens);
            break;
        case InputReader::COORD:
            nodeParser.parseCoords(node, tokens);
            break;
        case InputReader::QUALITY:
            nodeParser.parseInitQual(node, tokens);
            break;
        case InputReader::SOURCE:
            nodeParser.parseQualSource(node, network, tokens);
            break;
        case InputReader::MIXING:
            nodeParser.parseTankMixing(node, tokens);
            break;
        case InputReader::REACTION:
            nodeParser.parseTankReact(node, tokens);
            break;
    }
}

//-----------------------------------------------------------------------------

//  Read a link property from an input stream

void PropertyParser::parseLinkProperty(int type, string& linkName)
{
    Link* link = network->link(linkName);
    if ( link == nullptr ) throw InputError(InputError::UNDEFINED_OBJECT, linkName);
    switch (type)
    {
        case STATUS:  linkParser.parseStatus(link, tokens);               break;
        case LEAKAGE: linkParser.parseLeakage(link, tokens);              break;
        case ENERGY:  linkParser.parseEnergy(link, network, tokens);      break;
        case BULK:    linkParser.parseReaction(link, Link::BULK, tokens); break;
        case WALL:    linkParser.parseReaction(link, Link::WALL, tokens); break;
    }
}

//-----------------------------------------------------------------------------

//  Read a reaction property from an input stream

void PropertyParser::parseReactProperty(string& reactType)
{
    string name = tokens[1];
    if ( Utilities::match(reactType, w_Bulk) )
    {
        parseLinkProperty(BULK, name);
    }
    else if ( Utilities::match(reactType, w_Wall) )
    {
        parseLinkProperty(WALL, name);
    }
    else if ( Utilities::match(reactType, w_Tank) )
    {
        parseNodeProperty(InputReader::REACTION, name);
    }
    else optionParser.parseReactOption(network, tokens);
}

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "linkparser.h"
#include "Core/network.h"
#include "Core/constants.h"
#include "Core/error.h"
#include "Elements/pipe.h"
#include "Elements/pump.h"
#include "Elements/valve.h"
#include "Elements/pumpcurve.h"
#include "Utilities/utilities.h"

using namespace std;

//-----------------------------------------------------------------------------
//  Keywords associated with link properties
//-----------------------------------------------------------------------------
static const char* w_Head    = "HEAD";
static const char* w_Speed   = "SPEED";
static const char* w_Power   = "POWER";
static const char* w_Price   = "PRICE";
static const char* w_Pattern = "PATTERN";
static const char* w_Effic   = "EFFIC";
static const char* w_OPEN    = "OPEN";
static const char* w_CLOSED  = "CLOSED";
static const char* w_CV      = "CV";
static const char* valveTypeWords[] = {"PRV", "PSV", "FCV", "TCV", "PBV", "GPV", 0};

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void parseEndNodes(Link* link, Network* nw, vector<string>& tokens);
static void parsePipeData(Pipe* link, vector<string>& tokens);
static void parsePumpData(Pump* link, Network* nw, vector<string>& tokens);
static void parseValveData(Valve* link, Network* nw, vector<string>& tokens);

//-----------------------------------------------------------------------------

void LinkParser::parseLinkData(string& id, Network* nw, vector<string>& tokens)
{
    // ... read end nodes

    Link* link = nw->link(id);
    if ( !link ) throw InputError(InputError::UNDEFINED_OBJECT, id);
    if ( tokens.size() < 4 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    parseEndNodes(link, nw, tokens);

    // ... read link-specific data

    switch (link->type())
    {
    case Link::PIPE: parsePipeData(static_cast<Pipe*>(link), tokens);        break;
    case Link::PUMP: parsePumpData(static_cast<Pump*>(link), nw, tokens);    break;
    case Link::VALVE: parseValveData(static_cast<Valve*>(link), nw, tokens); break;
    default: throw InputError(InputError::UNDEFINED_OBJECT, id);
    }
}

//-----------------------------------------------------------------------------

void LinkParser::parseStatus(Link* link, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - link ID
    // 1 - OPEN/CLOSED keyword or numerical setting

    if ( tokenList.size() < 2 ) return;
    string* tokens = &tokenList[0];
    double setting;

    // ... check for OPEN/CLOSED keywords

    if (Utilities::match(tokens[1], "OPEN"))
    {
	    link->setInitStatus(Link::LINK_OPEN);
    }
    else if (Utilities::match(tokens[1], "CLOSED"))
    {
	    link->setInitStatus(Link::LINK_CLOSED);
    }

    // ... check for numerical setting value

    else if (Utilities::parseNumber(tokens[1], setting))
    {
	    link->setInitSetting(setting);
    }
    else throw InputError(InputError::INVALID_KEYWORD, tokens[1]);
}

//-----------------------------------------------------------------------------

void LinkParser::parseLeakage(Link* link, std::vector<std::string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - link ID
    // 1 - first leakage parameter
    // 2 - second leakage parameter
    // Parameters defined by user's choice of leakage model

    // ... cast link to a pipe (only pipe links have leakage parameters)

    if ( link->type() != Link::PIPE ) return;
    Pipe* pipe = static_cast<Pipe*>(link);

    // ... parse leakage parameters

    string* tokens = &tokenList[0];
    if ( !Utilities::parseNumber(tokens[1], pipe->leakCoeff1) )
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[1]);
    }
    if ( !Utilities::parseNumber(tokens[2], pipe->leakCoeff2) )
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[2]);
    }
}

//-----------------------------------------------------------------------------

void LinkParser::parseEnergy(
        Link* link, Network* network, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - PUMP keyword
    // 1 - pump ID
    // 2 - PRICE/PATTERN/EFFIC keyword
    // 3 - price value or ID of pattern or efficiency curve

    // ... cast link to a pump

    if ( link->type() != Link::PUMP ) return;
    Pump* pump = static_cast<Pump*>(link);

    // ... read keyword from input stream

    if ( tokenList.size() < 4) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];
    string keyword = tokens[2];

    // ... read energy cost per Kwh

    if ( Utilities::match(keyword, w_Price) )
    {
        if ( !Utilities::parseNumber(tokens[3], pump->costPerKwh) )
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[3]);
        }
    }

    // ... read name of energy cost time pattern

    else if ( Utilities::match(keyword, w_Pattern) )
    {
        pump->costPattern = network->pattern(tokens[3]);
        if ( !pump->costPattern )
        {
            throw InputError(InputError::UNDEFINED_OBJECT, tokens[3]);
        }
    }

    // ... read name of pump efficiency curve

    else if ( Utilities::match(keyword, w_Effic) )
    {
        pump->efficCurve = network->curve(tokens[3]);
        if ( !pump->efficCurve )
        {
            throw InputError(InputError::UNDEFINED_OBJECT, tokens[3]);
        }
    }

    else throw InputError(InputError::INVALID_KEYWORD, keyword);
}

//-----------------------------------------------------------------------------

void LinkParser::parseReaction(Link* link, int type, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - BULK/WALL keyword
    // 1 - link ID
    // 2 - reaction coeff. (1/days)

    // ... cast link to a pipe (only pipe links have reactions)

    if ( link->type() != Link::PIPE ) return;
    Pipe* pipe = static_cast<Pipe*>(link);

    // ... read reaction coeff.

    double x;
    string* tokens = &tokenList[0];
    if ( !Utilities::parseNumber(tokens[2], x) )
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[1]);
    }

    // ... save reaction coeff. converted to 1/sec

    if      (type == Link::BULK) pipe->bulkCoeff = x / SECperDAY;
    else if (type == Link::WALL) pipe->wallCoeff = x / SECperDAY;
}

//-----------------------------------------------------------------------------

void parseEndNodes(Link* link, Network* nw, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - link ID
    // 1 - start node ID
    // 2 - end node ID
    // remaining tokens are parsed by other functions

    string* tokens = &tokenList[0];

    // ... read index of link's start node

    link->fromNode = nw->node(tokens[1]);
    if ( link->fromNode == nullptr ) throw InputError(InputError::UNDEFINED_OBJECT, tokens[1]);

    // ... read end node

    link->toNode = nw->node(tokens[2]);
    if ( link->toNode == nullptr ) throw InputError(InputError::UNDEFINED_OBJECT, tokens[2]);
}

//-----------------------------------------------------------------------------

void parsePipeData(Pipe* pipe, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - pipe ID
    // 1 - start node ID
    // 2 - end node ID
    // 3 - length
    // 4 - diameter
    // 5 - roughness
    // 6 - minor loss coeff. (optional)
    // 7 - initial status (optional)

    int nTokens = tokenList.size();
    if ( nTokens < 6 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    // ... read length, diameter, and roughness

    double x[3];
    for (int i = 0; i < 3; i++)
    {
        if ( !Utilities::parseNumber(tokens[3+i], x[i]) || x[i] <= 0.0 )
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[3+i]);
        }
    }
    pipe->length    = x[0];
    pipe->diameter  = x[1];
    pipe->roughness = x[2];

    // ... read optional minor loss coeff.

    if ( nTokens > 6 )
    {
        if ( !Utilities::parseNumber(tokens[6], pipe->lossCoeff) ||
             pipe->lossCoeff < 0.0)
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[6]);
        }
    }

    // ... read optional initial status

    if ( nTokens > 7 && tokens[7] != "*" )
    {
        string s = tokens[7];
        if      (Utilities::match(s, w_OPEN))   pipe->initStatus = Link::LINK_OPEN;
        else if (Utilities::match(s, w_CLOSED)) pipe->initStatus = Link::LINK_CLOSED;
        else if (Utilities::match(s, w_CV))     pipe->hasCheckValve = true;
        else throw InputError(InputError::INVALID_KEYWORD, s);
    }
}

//-----------------------------------------------------------------------------

void parsePumpData(Pump* pump, Network* nw, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - pump ID
    // 1 - upstream node ID
    // 2 - downstream node ID
    // remaining tokens are property name/value pairs

    // ... start reading keyword/value pair tokens

    int nTokens = tokenList.size();
    string* tokens = &tokenList[0];
    int index = 3;
    string keyword;

    while ( index < nTokens )
    {
	// ... check that the value token exists

        keyword = tokens[index];
        index++;
        if ( index >= nTokens ) throw InputError(InputError::TOO_FEW_ITEMS, "");

        // ... horsepower property

        if ( Utilities::match(keyword, w_Power) )
        {
            if ( !Utilities::parseNumber(tokens[index], pump->pumpCurve.horsepower) )
            {
                throw InputError(InputError::INVALID_NUMBER, tokens[index]);
            }
        }

        // ... head curve property

        else if (Utilities::match(keyword, w_Head))
        {
            Curve* pumpCurve = nw->curve(tokens[index]);
            if ( !pumpCurve ) throw InputError(InputError::UNDEFINED_OBJECT, tokens[index]);
            pump->pumpCurve.curve = pumpCurve;
        }

        // ... speed setting property

        else if (Utilities::match(keyword, w_Speed))
        {
            if ( !Utilities::parseNumber(tokens[index], pump->speed)
                || pump->speed < 0.0 )
            {
                throw InputError(InputError::INVALID_NUMBER, tokens[index]);
            }
        }

        // ... speed pattern property

        else if (Utilities::match(keyword, w_Pattern))
        {
            pump->speedPattern = nw->pattern(tokens[index]);
            if ( !pump->speedPattern )
            {
                throw InputError(InputError::UNDEFINED_OBJECT, tokens[index]);
            }
        }

        else throw InputError(InputError::INVALID_KEYWORD, keyword);
        index++;
    }
}

//-----------------------------------------------------------------------------

void parseValveData(Valve* valve, Network* network, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - valve ID
    // 1 - upstream node ID
    // 2 - downstream node ID
    // 3 - diameter
    // 4 - valve type
    // 5 - valve setting
    // 6 - minor loss coeff. (optional)

    // ... check for enough input tokens

    if ( tokenList.size() < 6 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    // ... read diameter

    if ( !Utilities::parseNumber(tokens[3], valve->diameter)||
         valve->diameter <= 0.0 )
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[3]);
    }

    // ... read valve type

    int vType = Utilities::findMatch(tokens[4], valveTypeWords);
    if ( vType < 0 ) throw InputError(InputError::INVALID_KEYWORD, tokens[4]);
    valve->valveType = (Valve::ValveType)vType;

    // ... read index of head loss curve for General Purpose Valve

    if ( valve->valveType == Valve::GPV )
    {
        int c = network->indexOf(Element::CURVE, tokens[5]);
        if ( c < 0 ) throw InputError(InputError::UNDEFINED_OBJECT, tokens[5]);
        valve->initSetting = c;
    }

    // ... read numerical setting for other types of valves
    else
    {
        if ( !Utilities::parseNumber(tokens[5], valve->initSetting) )
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[5]);
        }
    }

    // ... read optional minor loss coeff.

    if ( tokenList.size() > 6 )
    {
        if ( !Utilities::parseNumber(tokens[6], valve->lossCoeff) ||
             valve->lossCoeff < 0.0 )
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[6]);
        }
    }
}

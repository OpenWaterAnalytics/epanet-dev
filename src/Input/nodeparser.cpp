/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "nodeparser.h"
#include "Core/network.h"
#include "Core/error.h"
#include "Elements/junction.h"
#include "Elements/reservoir.h"
#include "Elements/tank.h"
#include "Elements/demand.h"
#include "Elements/emitter.h"
#include "Elements/qualsource.h"
#include "Models/tankmixmodel.h"
#include "Utilities/utilities.h"

using namespace std;

//-----------------------------------------------------------------------------

static void parseJuncData(Junction* node, Network* nw, vector<string>& tokens);
static void parseResvData(Reservoir* node, Network* nw, vector<string>& tokens);
static void parseTankData(Tank* node, Network* nw, vector<string>& tokens);
static void parseDemandData(Demand& demand, Network* nw, vector<string>& tokens);
static void parseEmitterData(
               Network* nw, vector<string>& tokens, double& coeff,
               double& expon, Pattern* pattern);

//-----------------------------------------------------------------------------

void NodeParser::parseNodeData(string& id, Network* nw, vector<string>& tokens)
{
    Node* node = nw->node(id);
    if (node == nullptr) throw InputError(InputError::UNDEFINED_OBJECT, id);
    switch (node->type())
    {
    case Node::JUNCTION:  parseJuncData(static_cast<Junction*>(node), nw, tokens);  break;
    case Node::RESERVOIR: parseResvData(static_cast<Reservoir*>(node), nw, tokens); break;
    case Node::TANK:      parseTankData(static_cast<Tank*>(node), nw, tokens);      break;
    default:              throw InputError(InputError::UNDEFINED_OBJECT, id);
    }
}

//-----------------------------------------------------------------------------

void NodeParser::parseDemand(Node* node, Network* nw, vector<string>& tokens)
{
    // ... cast Node to Junction
    if ( tokens.size() < 2 ) return;
    if ( node->type() != Node::JUNCTION ) return;
    Junction* junc = static_cast<Junction*>(node);

    // ... declare a demand object and read its parameters
    Demand d;
    parseDemandData(d, nw, tokens);

    // ... add demand to junction
    //     (demand d passed by value so a copy is being added to demands list)
    junc->demands.push_back(d);
}

//-----------------------------------------------------------------------------

void NodeParser::parseEmitter(Node* node, Network* nw, vector<string>& tokens)
{
    // ... cast Node to Junction
    if ( tokens.size() < 2 ) return;
    if ( node->type() != Node::JUNCTION ) return;
    Junction* junc = static_cast<Junction*>(node);

    // ... read emitter's parameters
    double coeff = 0.0;
    double expon = nw->option(Options::EMITTER_EXPONENT);
    Pattern* pattern = 0;
    parseEmitterData(nw, tokens, coeff, expon, pattern);

    // ... add an emitter to the junction
    if ( !Emitter::addEmitter(junc, coeff, expon, pattern) )
    {
        throw InputError(InputError::CANNOT_CREATE_OBJECT, "Node Emitter");
    }
}

//-----------------------------------------------------------------------------

void NodeParser::parseCoords(Node* node, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - node ID
    // 1 - x coordinate
    // 2 - y coordinate

    if ( tokenList.size() < 3 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    if ( !Utilities::parseNumber(tokens[1], node->xCoord) )
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[1]);
    }
    if ( !Utilities::parseNumber(tokens[2], node->yCoord) )
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[2]);
    }
}

//-----------------------------------------------------------------------------

void NodeParser::parseInitQual(Node* node, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - node ID
    // 1 - initial quality concentration

    if ( tokenList.size() < 2 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    double x;
    if ( !Utilities::parseNumber(tokens[1], x) || x < 0.0)
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[1]);
    }
    node->initQual = x;
}

//-----------------------------------------------------------------------------

void NodeParser::parseQualSource(Node* node, Network* nw, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - node ID
    // 1 - source type keyword
    // 2 - baseline source strength
    // 3 - time pattern ID (optional)

    if ( tokenList.size() < 3 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    // ... read source type

    int t = Utilities::findMatch(tokens[1], QualSource::SourceTypeWords);
    if (t < 0) throw InputError(InputError::INVALID_KEYWORD, tokens[1]);

    // ... read baseline source strength

    double b;
    if ( !Utilities::parseNumber(tokens[2], b) )
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[2]);
    }

    // ... read optional pattern name

    Pattern* p = 0;
    if ( tokenList.size() > 3 && tokens[3] != "*")
    {
        p = nw->pattern(tokens[3]);
        if (p == nullptr) throw InputError(InputError::UNDEFINED_OBJECT, tokens[3]);
    }

    // ... add a water quality source to the node

    if ( !QualSource::addSource(node, t, b, p) )
    {
        throw InputError(InputError::CANNOT_CREATE_OBJECT, "Node Source");
    }
}

//-----------------------------------------------------------------------------

void NodeParser::parseTankMixing(Node* node, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - tank ID
    // 1 - name of mixing model option
    // 2 - mixing fraction (for MIX2 model only)

    if ( tokenList.size() < 2 ) return;
    string* tokens = &tokenList[0];

    // ... cast Node to Tank

    if ( node->type() != Node::TANK ) return;
    Tank* tank = static_cast<Tank*>(node);

    // ... read mixing model type

    int i = Utilities::findMatch(tokens[1], TankMixModel::MixingModelWords);
    if ( i < 0 ) throw InputError(InputError::INVALID_KEYWORD, tokens[1]);
    tank->mixingModel.type = i;

    // ... read mixing fraction for 2-compartment model

    if ( tank->mixingModel.type == TankMixModel::MIX2 )
    {
        if ( tokenList.size() < 3 ) throw InputError(InputError::TOO_FEW_ITEMS, "");
        double f;
        if ( !Utilities::parseNumber(tokens[2], f) || f < 0.0 || f > 1.0 )
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[2]);
        }
        else tank->mixingModel.fracMixed = f;
    }
}

//-----------------------------------------------------------------------------

void NodeParser::parseTankReact(Node* node, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - TANK keyword
    // 1 - tank ID
    // 2 - reaction coeff. (1/days)

    if ( tokenList.size() < 2 ) return;
    string* tokens = &tokenList[0];

    // ... cast Node to Tank

    if ( node->type() != Node::TANK ) return;
    Tank* tank = static_cast<Tank*>(node);

    // ... read reaction coefficient in 1/days

    if ( !Utilities::parseNumber(tokens[1], tank->bulkCoeff) )
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[1]);
    }

    // ... convert coefficient to 1/sec

    tank->bulkCoeff /= 86400;
}

//-----------------------------------------------------------------------------

void parseJuncData(Junction* junc, Network* nw, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - junction ID
    // 1 - elevation
    // 2 - primary base demand (optional)
    // 3 - ID of primary demand pattern (optional)

    // ... check for enough tokens

    int nTokens = tokenList.size();
    if (nTokens < 2) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    // ... read elevation

    if (!Utilities::parseNumber(tokens[1], junc->elev))
    {
	    throw InputError(InputError::INVALID_NUMBER, tokens[1]);
    }

    // ... read optional base demand

    if (nTokens > 2)
	if (!Utilities::parseNumber(tokens[2], junc->primaryDemand.baseDemand))
	{
        throw InputError(InputError::INVALID_NUMBER, tokens[2]);
	}

    // ... read optional demand pattern

    if (nTokens > 3 && tokens[3] != "*")
    {
        junc->primaryDemand.timePattern = nw->pattern(tokens[3]);
        if ( !junc->primaryDemand.timePattern )
        {
            throw InputError(InputError::UNDEFINED_OBJECT, tokens[3]);
        }
    }
}

//-----------------------------------------------------------------------------

void parseResvData(Reservoir* resv, Network* nw, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - reservoir ID
    // 1 - water surface elevation
    // 2 - water surface elevation pattern (optional)

    // ... check for enough tokens

    int nTokens = tokenList.size();
    if (nTokens < 2) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    // ... read elevation

    if (!Utilities::parseNumber(tokens[1], resv->elev))
    {
	    throw InputError(InputError::INVALID_NUMBER, tokens[1]);
    }

    // read optional elevation pattern

    if ( nTokens > 2 && tokens[2] != "*")
    {
        resv->headPattern = nw->pattern(tokens[2]);
        if ( !resv->headPattern )
        {
            throw InputError(InputError::UNDEFINED_OBJECT, tokens[2]);
        }
    }
}

//-----------------------------------------------------------------------------

void parseTankData(Tank* tank, Network* nw, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - tank ID
    // 1 - elevation of bottom of tank bowl
    // 2 - initial water depth
    // 3 - minimum water depth
    // 4 - maximum water depth
    // 5 - nominal diameter
    // 6 - volume at minimum water depth
    // 7 - ID of volume v. depth curve (optional)

    // ... check for enough tokens

    int nTokens = tokenList.size();
    if (nTokens < 7) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string* tokens = &tokenList[0];

    // .... read 6 numbers from input stream into buffer x

    double x[6];
    for (int i = 0; i < 6; i++)
    {
        if (!Utilities::parseNumber(tokens[i+1], x[i]))
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[i+1]);
        }
    }

    // ... extract tank properties from buffer

    tank->elev      = x[0];
    tank->initHead  = tank->elev + x[1];    // convert input water depths to heads
    tank->minHead   = tank->elev + x[2];
    tank->maxHead   = tank->elev + x[3];
    tank->diameter  = x[4];
    tank->minVolume = x[5];

    // ... read optional volume curve

    if (nTokens > 7 && tokens[7] != "*")
    {
        tank->volCurve = nw->curve(tokens[7]);
        if ( !tank->volCurve )
        {
            throw InputError(InputError::UNDEFINED_OBJECT, tokens[7]);
        }
    }
}

//-----------------------------------------------------------------------------

void parseEmitterData(
        Network* nw,
        vector<string>& tokenList,
        double& coeff,
        double& expon,
        Pattern* pattern)
{
    // Contents of tokenList are:
    // 0 - junction ID
    // 1 - flow coefficient
    // 2 - flow exponent (optional)
    // 3 - ID of flow coeff. time pattern (optional)

    int nTokens = tokenList.size();
    string* tokens = &tokenList[0];

    // ... read flow coefficient

    if ( !Utilities::parseNumber(tokens[1], coeff) )
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[1]);
    }

    // ... read pressure exponent if present

    if ( nTokens > 2 )
    {
        if ( !Utilities::parseNumber(tokens[2], expon) )
        {
            throw InputError(InputError::INVALID_NUMBER, tokens[2]);
        }
    }

    // ... read optional time pattern

    if ( nTokens > 3 && tokens[3] != "*" )
    {
        pattern = nw->pattern(tokens[3]);
        if ( !pattern )
        {
            throw InputError(InputError::UNDEFINED_OBJECT, tokens[3]);
        }
    }
}

//-----------------------------------------------------------------------------

void parseDemandData(Demand& demand, Network* nw, vector<string>& tokenList)
{
    // Contents of tokenList are:
    // 0 - junction ID
    // 1 - base demand
    // 2 - ID of demand pattern (optional)

    int nTokens = tokenList.size();
    if ( nTokens < 2 ) return;
    string* tokens = &tokenList[0];

    // ... read base demand

    if ( !Utilities::parseNumber(tokens[1], demand.baseDemand) )
    {
        throw InputError(InputError::INVALID_NUMBER, tokens[1]);
    }

    // ... read optional demand pattern

    if ( nTokens > 2 && tokens[2] != "*" )
    {
        demand.timePattern = nw->pattern(tokens[2]);
        if ( !demand.timePattern )
        {
            throw InputError(InputError::UNDEFINED_OBJECT, tokens[2]);
        }
    }
}

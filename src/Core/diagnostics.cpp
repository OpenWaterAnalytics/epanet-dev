/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

 ////////////////////////////////////////////////
 //  Implementation of the Diagnostics class.  //
 ////////////////////////////////////////////////

 // TO DO:
 // - complete the validation of valve connections
 // - write the hasValidCurves() function
 // - add validation for variable patterns
 // - add diagnostics for output results (e.g., check for negative pressures
 //   and isolated nodes)

#include "diagnostics.h"
#include "Core/network.h"
#include "Core/error.h"
#include "Elements/node.h"
#include "Elements/valve.h"

#include <vector>
using namespace std;

bool hasValidNodes(Network* nw);
bool hasFixedGradeNodes(Network* nw);
bool hasValidLinks(Network* nw);
bool hasValidValves(Network* nw);
bool hasValidCurves(Network* nw);
bool hasConnectedNodes(Network* nw);

//-----------------------------------------------------------------------------

void Diagnostics::validateNetwork(Network* nw)
{
    bool result = true;
    if ( nw->count(Element::NODE) < 2 )
    {
        throw NetworkError(NetworkError::TOO_FEW_NODES, "");
    }
    if ( !hasFixedGradeNodes(nw) )
    {
         throw NetworkError(NetworkError::NO_FIXED_GRADE_NODES, "");
    }
    if ( !hasValidNodes(nw) )      result = false;
    if ( !hasValidLinks(nw) )      result = false;
    if ( !hasValidValves(nw) )     result = false;
    if ( !hasValidCurves(nw) )     result = false;
    if ( !hasConnectedNodes(nw) )  result = false;
    if ( result == false )
    {
        throw InputError(InputError::ERRORS_IN_INPUT_DATA, "");
    }
}

//-----------------------------------------------------------------------------

bool hasFixedGradeNodes(Network* nw)
{
    for (Node* node : nw->nodes )
    {
        if ( node->fixedGrade ) return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

bool hasValidNodes(Network* nw)
{
    bool result = true;
    for (Node* node : nw->nodes )
    {
        try
        {
            node->validate(nw);
        }
        catch (ENerror const& e)
        {
            nw->msgLog << e.msg;
            result = false;
        }
    }
    return result;
}

//-----------------------------------------------------------------------------

bool hasValidLinks(Network* nw)
{
    bool result = true;
    for (Link* link : nw->links)
    {
        try
        {
            link->validate(nw);
        }
        catch (ENerror const& e)
        {
            nw->msgLog << e.msg;
            result = false;
        }
    }
    return result;
}

//-----------------------------------------------------------------------------

bool hasValidValves(Network* nw)
{
    bool result = true;
    for (Link* link : nw->links)
    {
        if ( link->type() != Link::VALVE ) continue;
        try
        {
            Valve* valve = static_cast<Valve*>(link);
            if ( (link->toNode->fixedGrade && valve->valveType == Valve::PRV) ||
                 (link->fromNode->fixedGrade && valve->valveType == Valve::PSV) )
            {
                throw NetworkError(NetworkError::ILLEGAL_VALVE_CONNECTION,
                                   valve->name);
            }
        }
        catch (ENerror const& e)
        {
            nw->msgLog << e.msg;
            result = false;
        }

//////////  TO DO: Add checks for valves in series, etc.  ////////////////

    }
    return result;

}

//-----------------------------------------------------------------------------

bool hasValidCurves(Network* nw)
{
////////  TO BE ADDED  ////////
    return true;
}

//-----------------------------------------------------------------------------

bool hasConnectedNodes(Network* nw)
{
    int nodeCount = nw->count(Element::NODE);
    vector<char> marked(nodeCount, 0);
    for (Link* link : nw->links)
    {
        marked[link->fromNode->index]++;
        marked[link->toNode->index]++;
    }

    int unmarkedCount = 0;
    for (int i = 0; i < nodeCount; i++)
    {
        try
        {
            if ( !marked[i] )
            {
                unmarkedCount++;
                if ( unmarkedCount <= 10 )
                    throw NetworkError(
                        NetworkError::UNCONNECTED_NODE, nw->node(i)->name);
            }
        }
        catch (ENerror const& e)
        {
            nw->msgLog << e.msg;
        }
    }
    if ( unmarkedCount > 10 )
    {
        nw->msgLog << "\n\n NETWORK ERROR 233: no links connected to another "
                   << (unmarkedCount - 10) << " nodes ";
    }
    return (unmarkedCount == 0);
}

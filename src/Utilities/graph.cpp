/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

 //////////////////////////////////////////
 //  Implementation of the Graph class.  //
 //////////////////////////////////////////

 // TO DO:
 // - add network connectivity checking to support diagnostics.cpp
 // - add a topolgical sort routine to support qualengine.cpp

#include "graph.h"
#include "Core/network.h"
#include "Elements/link.h"
#include "Elements/node.h"

#include <vector>
using namespace std;

//-----------------------------------------------------------------------------

//  Constructor/Destructor

Graph::Graph()
{
}

Graph::~Graph()
{
}

//-----------------------------------------------------------------------------

void Graph::createAdjLists(Network* nw)
{
    try
    {
        int nodeCount = nw->count(Element::NODE);
        int linkCount = nw->count(Element::LINK);
        adjLists.resize(2*linkCount, -1);
        adjListBeg.resize(nodeCount+1, 0);

        vector<int> degree(nodeCount, 0);
        for (Link* link : nw->links)
        {
            degree[link->fromNode->index]++;
            degree[link->toNode->index]++;
        }
        adjListBeg[0] = 0;
        for (int i = 0; i < nodeCount; i++)
        {
            adjListBeg[i+1] = adjListBeg[i] + degree[i];
            degree[i] = 0;
        }

        int m;
        for (int k = 0; k < linkCount; k++)
        {
            int i = nw->link(k)->fromNode->index;
            m = adjListBeg[i] + degree[i];
            adjLists[m] = k;
            degree[i]++;
            int j = nw->link(k)->toNode->index;
            m = adjListBeg[j] + degree[j];
            adjLists[m] = k;
            degree[j]++;
        }
    }
    catch (...)
    {
        throw;
    }
}

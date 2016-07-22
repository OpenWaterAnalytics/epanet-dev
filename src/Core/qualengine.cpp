/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

 ///////////////////////////////////////////////
 //  Implementation of the QualEngine class.  //
 ///////////////////////////////////////////////

 // TO DO:
 // - implement a topological sort in function sortLinks()

#include "qualengine.h"
#include "network.h"
#include "error.h"
#include "Models/qualmodel.h"
#include "Solvers/qualsolver.h"
#include "Elements/qualsource.h"
#include "Elements/tank.h" // includes node.h
#include "Elements/link.h"
#include "Utilities/utilities.h"

#include <cmath>
#include <algorithm>
using namespace std;

//-----------------------------------------------------------------------------

//  Constructor

QualEngine::QualEngine() :
    engineState(QualEngine::CLOSED),
    network(nullptr),
    qualSolver(nullptr),
    nodeCount(0),
    linkCount(0),
    qualTime(0),
    qualStep(0)
{
}

//-----------------------------------------------------------------------------

//  Destructor

QualEngine::~QualEngine()
{
    close();

//    cout << "\nQualEngine destructed.\n";
}

//-----------------------------------------------------------------------------

//  Open the water quality engine.

void QualEngine::open(Network* nw)
{
    // ... close currently open engine

    if (engineState != QualEngine::CLOSED) close();

    // ... assign network to engine

    network = nw;
    nodeCount = network->count(Element::NODE);
    linkCount = network->count(Element::LINK);

    // ... create a water quality reaction model

    network->createQualModel();

    // ... no quality solver if there's no network quality model

    if ( network->qualModel == nullptr )
    {
        qualSolver = nullptr;
        return;
    }

    // ... create a water quality solver

    qualSolver = QualSolver::factory("LTD", network);
    if (!qualSolver) throw SystemError(SystemError::QUALITY_SOLVER_NOT_OPENED);

    // ... create sorted link & flow direction arrays

    try
    {
	    sortedLinks.resize(linkCount, 0);
        flowDirection.resize(linkCount, 0);
        engineState = QualEngine::OPENED;
    }
    catch (...)
    {
        throw SystemError(SystemError::QUALITY_SOLVER_NOT_OPENED);
    }
}

//-----------------------------------------------------------------------------

//  Initialize the water quality engine.

void QualEngine::init()
{
    if (engineState != QualEngine::OPENED) return;

    // ... initialize node concentrations & tank volumes

    for (Node* node : network->nodes)
    {
        if ( network->qualModel->type == QualModel::TRACE ) node->quality = 0.0;
        else node->quality = node->initQual;
        if ( node->type() == Node::TANK )
        {
            Tank* tank = static_cast<Tank*>(node);
            tank->volume = tank->findVolume(tank->initHead);
        }
    }

    // ... initialize reaction model and quality solver

    qualSolver->init();
    network->qualModel->init(network);
    qualStep = network->option(Options::QUAL_STEP);
    if ( qualStep <= 0 ) qualStep = 300;
    qualTime = 0;
    engineState = QualEngine::INITIALIZED;
}

//-----------------------------------------------------------------------------

//  Solve for water quality over current time step.

void QualEngine::solve(int tstep)
{
    // ... check that engine has been initialized

    if ( engineState != QualEngine::INITIALIZED ) return;
    if ( tstep == 0 ) return;

   // ... topologically sort the links if flow direction has changed

    if ( qualTime == 0 ) sortLinks();
    else if ( flowDirectionsChanged() ) sortLinks();

    // ... determine external source quality

    setSourceQuality();

    // ... propagate water quality through network over a sequence
    //     of water quality time steps

    qualTime += tstep;

    while ( tstep > 0 )
    {
        int qstep = min(qualStep, tstep);
        qualSolver->solve(&sortedLinks[0], qstep);
        tstep -= qstep;
    }
}

//-----------------------------------------------------------------------------

//  Close the quality solver.

void QualEngine::close()
{
    delete qualSolver;
    qualSolver = nullptr;
    sortedLinks.clear();
    flowDirection.clear();
    engineState = QualEngine::CLOSED;
}

//-----------------------------------------------------------------------------

//  Check if the flow direction of any link has changed.

bool QualEngine::flowDirectionsChanged()
{
    bool result = false;
    for (int i = 0; i < linkCount; i++)
    {
        if ( network->link(i)->flow * flowDirection[i] < 0 )
        {
            qualSolver->reverseFlow(i);
            result = true;
        }
    }
    return result;
}

//-----------------------------------------------------------------------------

//  Compute the quality entering the network from each source node.

void QualEngine::setSourceQuality()
{
    // ... set source strength for each source node

    int sourceCount = 0;
    for (Node* node : network->nodes)
    {
        if ( node->qualSource )
        {
            node->qualSource->setStrength(node);
            node->qualSource->outflow = 0.0;
            sourceCount++;
        }
    }
    if ( sourceCount == 0 ) return;

    // ... find flow rate leaving each source node

    Node* fromNode;
    for (Link* link : network->links)
    {
        double q = link->flow;
        if ( q >= 0.0 ) fromNode = link->fromNode;
        else            fromNode = link->toNode;
        if ( fromNode->qualSource ) fromNode->qualSource->outflow += abs(q);
    }
}

//-----------------------------------------------------------------------------

//  Topologically sort the network's links based on current flow directions.

void QualEngine::sortLinks()
{
    // ... default sorted order

    setFlowDirections();
    for (int j = 0; j < linkCount; j++) sortedLinks[j] = j;
}

//-----------------------------------------------------------------------------

//  Set the flow direction indicator in each network link.

void QualEngine::setFlowDirections()
{
    for (int i = 0; i < linkCount; i++)
    {
    	flowDirection[i] = Utilities::sign(network->link(i)->flow);
    }
}

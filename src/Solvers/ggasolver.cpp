/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

 ////////////////////////////////////////////////////////////////////////
 //  Implementation of the Global Gradient Algorithm hydraulic solver  //
 ////////////////////////////////////////////////////////////////////////

 // TO DO:
 // - implement the line search procedure and consider moving it to
 //   its own module so it can be used by other solvers

#include "ggasolver.h"
#include "matrixsolver.h"
#include "Core/network.h"
#include "Core/constants.h"
#include "Elements/junction.h"
#include "Elements/tank.h"
#include "Elements/link.h"

#include <cstring>
#include <cmath>
#include <limits>
#include <iostream>   //for debugging
#include <iomanip>
#include <algorithm>
using namespace std;

static const string s_Trial          = "    Trial ";
static const string s_IllConditioned = "  Hydraulic matrix ill-conditioned at node ";
static const string s_StepSize       = "    Step Size   = ";
static const string s_TotalError     = "    Error Norm  = ";
static const string s_HlossEvals     = "    Head Loss Evaluations = ";
static const string s_HeadError      = "    Head Error  = ";
static const string s_ForLink        = " for Link ";
static const string s_FlowError      = "    Flow Error  = ";
static const string s_AtNode         = " at Node ";
static const string s_FlowChange     = "    Flow Change = ";
static const string s_TotFlowChange  = "    Total Flow Change Ratio = ";
static const string s_NodeLabel      = "  Node ";
static const string s_FGChange       = "    Fixed Grade Status changed to ";

//-----------------------------------------------------------------------------

// error norm threshold for status checking
static const double ErrorThreshold = 1.0;
static const double Huge = numeric_limits<double>::max();

// step sizing enumeration
enum StepSizing {FULL, RELAXATION, LINESEARCH};

//-----------------------------------------------------------------------------

//  Constructor

GGASolver::GGASolver(Network* nw, MatrixSolver* ms) : HydSolver(nw, ms)
{
    nodeCount = network->count(Element::NODE);
    linkCount = network->count(Element::LINK);

    dH.resize(nodeCount, 0);           // nodal head changes
    dQ.resize(linkCount, 0);           // link flow changes
    xQ.resize(nodeCount, 0);           // nodal excess flow (inflow - outflow)

    hLossEvalCount  = 0;
    trialsLimit     = 0;
    reportTrials    = network->option(Options::REPORT_TRIALS);

    headErrLimit    = 0.0;
    flowErrLimit    = 0.0;
    flowChangeLimit = 0.0;
    flowRatioLimit  = 0.0;
    tstep           = 0.0;
    theta           = 0.0;

    if (network->option(Options::STEP_SIZING) == "RELAXATION" )
        stepSizing = RELAXATION;
    else if (network->option(Options::STEP_SIZING) == "LINESEARCH" )
        stepSizing = LINESEARCH;
    else stepSizing = FULL;

    errorNorm     = 0.0;
    oldErrorNorm  = 0.0;
}

//-----------------------------------------------------------------------------

//  Destructor

GGASolver::~GGASolver()
{
    dH.clear();
    dQ.clear();
    xQ.clear();
}

//-----------------------------------------------------------------------------

//  Solve network for heads and flows

int GGASolver::solve(double tstep_, int& trials)
{
    // ... initialize variables

    double lamda = 1.0;
    bool statusChanged = true;
    bool converged = false;

    errorNorm = Huge;
    hLossEvalCount = 0;
    tstep = tstep_;
    trials = 1;

    // ... get time weighting option for tank updating

    theta = network->option(Options::TIME_WEIGHT);
    theta = min(theta, 1.0);
    if ( theta > 0.0 ) theta = max(theta, 0.5);

    // ... set values for convergence limits

    setConvergenceLimits();

    // ... perform Newton iterations

    while ( trials <= trialsLimit )
    {
        // ... save current error norm

        oldErrorNorm = errorNorm;

        // ... determine which nodes have fixed heads (e.g., PRVs)

        setFixedGradeNodes();

        // ... re-compute error norm if links change status

        if ( statusChanged )
        {
            oldErrorNorm = findErrorNorm(0.0);
            lamda = 1.0;
        }
        statusChanged = false;

        // ... find changes in heads and flows

        int errorCode = findHeadChanges();
        if ( errorCode >= 0 )
        {
       	    Node* node = network->node(errorCode);
            network->msgLog << endl << s_IllConditioned << node->name;
            return HydSolver::FAILED_ILL_CONDITIONED;
        }
        findFlowChanges();

        // ... find step size to take for head/flow changes
        //     (which evaluates new gradients for next trial)

        lamda = findStepSize(trials);
        updateSolution(lamda);

        // ... check for convergence

        if ( reportTrials ) reportTrial(trials, lamda);
        converged = hasConverged();

        // ... if close to convergence then check for any link status changes

        if ( converged ) //|| errorNorm < ErrorThreshold )
        {
            statusChanged = linksChangedStatus();
        }

        // ... check if the current solution can be accepted

        if ( converged && !statusChanged ) break;
        trials++;
    }
    //if ( reportTrials ) network->msgLog << s_HlossEvals << hLossEvalCount;
    if ( trials > trialsLimit ) return HydSolver::FAILED_NO_CONVERGENCE;
    return HydSolver::SUCCESSFUL;
}

//-----------------------------------------------------------------------------

//  Establish error limits for convergence of heads and flows

void GGASolver::setConvergenceLimits()
{
    // ... maximum trials
    trialsLimit = network->option(Options::MAX_TRIALS);

    // ... limit on relative flow change ratio (old EPANET2 criterion)
    flowRatioLimit = network->option(Options::RELATIVE_ACCURACY);

    // ... tolerance in satisfying hydraulic balances
    headErrLimit = network->option(Options::HEAD_TOLERANCE) /
                   network->ucf(Units::LENGTH);
    flowErrLimit = network->option(Options::FLOW_TOLERANCE) /
                   network->ucf(Units::FLOW);

    // ... limit on largest flow change
    flowChangeLimit = network->option(Options::FLOW_CHANGE_LIMIT) /
                      network->ucf(Units::FLOW);

    // ... use a default head error limit if need be
    if ( flowRatioLimit == 0.0 && headErrLimit == 0.0 &&
         flowErrLimit == 0.0 && flowChangeLimit == 0.0 )
    {
        headErrLimit = 0.005;
    }

    // ... convert missing limits to a huge number
    if ( flowRatioLimit  == 0.0 ) flowRatioLimit  = Huge;
    if ( headErrLimit    == 0.0 ) headErrLimit    = Huge;
    if ( flowErrLimit    == 0.0 ) flowErrLimit    = Huge;
    if ( flowChangeLimit == 0.0 ) flowChangeLimit = Huge;}

//-----------------------------------------------------------------------------

//  Adjust fixed grade status of specific nodes.

void GGASolver::setFixedGradeNodes()
{
    Node* node;

    // ... change fixed grade status for PRV/PSV nodes

    for (Link* link : network->links)
    {
        // ... check if link is a PRV or PSV

        if      ( link->isPRV() ) node = link->toNode;
        else if ( link->isPSV() ) node = link->fromNode;
        else continue;

        // ... set the fixed grade status of the valve's control node

        if ( link->status == Link::VALVE_ACTIVE )
        {
            node->fixedGrade = true;
            node->head = link->setting + node->elev;
        }
        else node->fixedGrade = false;
    }

    // ... after time 0, tstep will be non-zero and tank levels
    //     will be non-fixed if time weighting (theta) is non-zero

    if ( theta > 0.0 && tstep > 0.0 )
    {
        for (Node* tankNode : network->nodes)
        {
            if ( tankNode->type() == Node::TANK ) tankNode->fixedGrade = false;
        }
    }

}

//-----------------------------------------------------------------------------

//  Find changes in nodal heads by solving a linearized system of equations.

int GGASolver::findHeadChanges()
{
    // ... setup the coeff. matrix of the GGA linearized system

    setMatrixCoeffs();

    // ... temporarily use the head change array dH[] to store new heads

    double *h = &dH[0];

    // ... solve the linearized GGA system for new nodal heads
    //     (matrixSolver returns a negative integer if it runs successfully;
    //      otherwise it returns the index of the row that caused it to fail.)

    int errorCode = matrixSolver->solve(nodeCount, h);
    if ( errorCode >= 0 ) return errorCode;

    // ... save new heads as head changes

    for (int i = 0; i < nodeCount; i++)
    {
        dH[i] = h[i] - network->node(i)->head;
    }

    // ... return a negative number indicating that
    //     the matrix solver ran successfully

     return -1;
}

//-----------------------------------------------------------------------------

//  Find the changes in link flows resulting from a set of nodal head changes.

void GGASolver::findFlowChanges()
{
    for (int i = 0; i < linkCount; i++)
    {
        // ... get link object and its end node indexes

        dQ[i] = 0.0;
        Link* link = network->link(i);
        int n1 = link->fromNode->index;
        int n2 = link->toNode->index;

        // ... flow change for pressure regulating valves

        if ( link->hGrad == 0.0 )
        {
            if ( link->isPRV() ) dQ[i] = -xQ[n2] - link->flow;
            if ( link->isPSV() ) dQ[i] = xQ[n1] - link->flow;
            continue;
        }

        // ... apply GGA flow change formula:

        double dh = (link->fromNode->head + dH[n1]) -
                    (link->toNode->head + dH[n2]);
        double dq = (link->hLoss - dh) / link->hGrad;

        // ... special case to prevent negative flow in constant HP pumps

        if ( link->isHpPump() &&
             link->status == Link::LINK_OPEN &&
             dq > link->flow ) dq = link->flow / 2.0;

        // ... save flow change

        dQ[i] = -dq;
    }
}

//-----------------------------------------------------------------------------

//  Find how much of the head and flow changes to apply to a new solution.

double GGASolver::findStepSize(int trials)
{
    // ... find the new error norm at full step size

    double lamda = 1.0;
    errorNorm = findErrorNorm(lamda);

    if ( stepSizing == RELAXATION && oldErrorNorm < ErrorThreshold )
    {
        lamda = 0.5;
        double errorNorm2 = findErrorNorm(lamda);
        if ( errorNorm2 < errorNorm ) errorNorm = errorNorm2;
        else
        {
            lamda = 1.0;
            errorNorm = findErrorNorm(lamda);
        }
    }

    // ... if called for, implement a line search procedure
    //     to find the best step size lamda to take

    if ( stepSizing == LINESEARCH && trials > 1 )
    {
    ////////////  TO BE IMPLEMENTED  ///////////////
    }
    return lamda;
}

//-----------------------------------------------------------------------------

//  Compute the error norm associated with a given step size.

double GGASolver::findErrorNorm(double lamda)
{
    hLossEvalCount++;
    return hydBalance.evaluate(lamda, (double*)&dH[0], (double*)&dQ[0],
                                      (double*)&xQ[0], network);
}

//-----------------------------------------------------------------------------

//  Update heads and flows for a given step size.

void GGASolver::updateSolution(double lamda)
{
    for (int i = 0; i < nodeCount; i++) network->node(i)->head += lamda * dH[i];
    for (int i = 0; i < linkCount; i++) network->link(i)->flow += lamda * dQ[i];
}

//-----------------------------------------------------------------------------

//  Check if current solution meets convergence limits

bool GGASolver::hasConverged()
{
    return ( hydBalance.maxHeadErr < headErrLimit ) &&
           ( hydBalance.maxFlowErr < flowErrLimit ) &&
           ( hydBalance.maxFlowChange < flowChangeLimit ) &&
           ( hydBalance.totalFlowChange < flowRatioLimit );
}

//-----------------------------------------------------------------------------

void GGASolver::reportTrial(int trials, double lamda)
{
    network->msgLog << endl << endl << s_Trial << trials << ":";
    network->msgLog << endl << s_StepSize << lamda;
    network->msgLog << endl << s_TotalError << errorNorm;

    // ... report link with maximum head loss error

    network->msgLog << endl << s_HeadError <<
        hydBalance.maxHeadErr * network->ucf(Units::LENGTH) << " " <<
        network->getUnits(Units::LENGTH);
    if (hydBalance.maxHeadErrLink >= 0)
    {
        network->msgLog << s_ForLink << network->link(hydBalance.maxHeadErrLink)->name;
    }

    // ... report node with maximum flow balance error

    network->msgLog << endl << s_FlowError <<
        hydBalance.maxFlowErr * network->ucf(Units::FLOW) <<
		" " << network->getUnits(Units::FLOW);
    if (hydBalance.maxFlowErrNode >= 0)
    {
        network->msgLog << s_AtNode << network->node(hydBalance.maxFlowErrNode)->name;
    }

    // ... report link with largest change in flow rate

    network->msgLog << endl << s_FlowChange << hydBalance.maxFlowChange *
	    network->ucf(Units::FLOW) << " " << network->getUnits(Units::FLOW);
    if ( hydBalance.maxFlowChangeLink >= 0 )
    {
        network->msgLog << s_ForLink << network->link(hydBalance.maxFlowChangeLink)->name;
    }

    // ... report total link flow change relative to total link flow

    network->msgLog << endl << s_TotFlowChange << hydBalance.totalFlowChange;
}

//-----------------------------------------------------------------------------

//  Compute the coeffciient matrix of the linearized set of equations for heads.

void GGASolver::setMatrixCoeffs()
{
    memset(&xQ[0], 0, nodeCount*sizeof(double));
    matrixSolver->reset();
    setLinkCoeffs();
    setNodeCoeffs();
    setValveCoeffs();
}

//-----------------------------------------------------------------------------

//  Compute matrix coefficients for link head loss gradients.

void GGASolver::setLinkCoeffs()
{
    for (int j = 0; j < linkCount; j++)
    {
        // ... skip links with zero head gradient
        //     (e.g. active pressure regulating valves)

        Link* link = network->link(j);
        if ( link->hGrad == 0.0 ) continue;

        // ... identify end nodes of link

        Node* node1 = link->fromNode;
        Node* node2 = link->toNode;
        int n1 = node1->index;
        int n2 = node2->index;

        // ... update node flow balances

        xQ[n1] -= link->flow;
        xQ[n2] += link->flow;

        // ... a is contribution to coefficient matrix
        //     b is contribution to right hand side

        double a = 1.0 / link->hGrad;
        double b = a * link->hLoss;

        // ... update off-diagonal coeff. of matrix if both start and
        //     end nodes are not fixed grade

        if ( !node1->fixedGrade && !node2->fixedGrade )
        {
            matrixSolver->addToOffDiag(j, -a);
        }

        // ... if start node has fixed grade, then apply a to r.h.s.
        //     of that node's row;

        if ( node1->fixedGrade )
        {
            matrixSolver->addToRhs(n2, a * node1->head);
        }

        // ... otherwise add a to row's diagonal coeff. and
        //     add b to its r.h.s.

        else
        {
            matrixSolver->addToDiag(n1, a);
            matrixSolver->addToRhs(n1, b);
        }

        // ... do the same for the end node, except subtract b from r.h.s

        if ( node2->fixedGrade )
        {
            matrixSolver->addToRhs(n1, a * node2->head);
        }
        else
        {
            matrixSolver->addToDiag(n2, a);
            matrixSolver->addToRhs(n2, -b);
        }
    }
}

//-----------------------------------------------------------------------------

//  Compute matrix coefficients for dynamic tanks and external node outflows.

void  GGASolver::setNodeCoeffs()
{
    for (int i = 0; i < nodeCount; i++)
    {
        // ... if node's head not fixed

        Node* node = network->node(i);
        if ( !node->fixedGrade )
        {
            // ... for dynamic tanks, add area terms to row i
            //     of the head solution matrix & r.h.s. vector

            if ( node->type() == Node::TANK && theta != 0.0 )
            {
                Tank* tank = static_cast<Tank*>(node);
                double a = tank->area / (theta * tstep);
                matrixSolver->addToDiag(i, a);

                a = a * tank->pastHead + (1.0 - theta) * tank->pastOutflow / theta;
                matrixSolver->addToRhs(i, a);
            }

            // ... for junctions, add effect of external outflows

            else if ( node->type() == Node::JUNCTION )
            {
                // ... update junction's net inflow
                xQ[i] -= node->outflow;
                matrixSolver->addToDiag(i, node->qGrad);
                matrixSolver->addToRhs(i, node->qGrad * node->head);
            }

            // ... add node's net inflow to r.h.s. row
            matrixSolver->addToRhs(i, (double)xQ[i]);
        }

        // ... if node has fixed head, force solution to produce it

        else
        {
            matrixSolver->setDiag(i, 1.0);
            matrixSolver->setRhs(i, node->head);
        }
    }
}

//-----------------------------------------------------------------------------

//  Compute matrix coefficients for pressure regulating valves.

void  GGASolver::setValveCoeffs()
{
    for (Link* link : network->links)
    {
        // ... skip links that are not active pressure regulating valves

        if ( link->hGrad > 0.0 ) continue;

        // ... determine end node indexes of link

        int n1 = link->fromNode->index;
        int n2 = link->toNode->index;

        // ... add net inflow of downstream node of a PRV to the
        //     r.h.s. row of its upstream node

        if ( link->isPRV() )
        {
            matrixSolver->addToRhs(n1, (double)xQ[n2]);
        }

        // ... add net inflow of upstream node of a PSV to the
        //     r.h.s. row of its downstream node

        if ( link->isPSV() )
        {
            matrixSolver->addToRhs(n2, (double)xQ[n1]);
        }
    }
}

//-----------------------------------------------------------------------------

//  Check if any links change status at the current trial solution.

bool GGASolver::linksChangedStatus()
{
    bool result = false;
    for (Link* link : network->links)
    {
        // ... get head at each end of link

        double h1 = link->fromNode->head;
        double h2 = link->toNode->head;
        double q = link->flow;

        // ... update link's status

        int oldStatus = link->status;
        if ( link->status >= Link::TEMP_CLOSED ) link->status = Link::LINK_OPEN;
        link->updateStatus(q, h1, h2);

        // ... check for flow into full or out of empty tanks

        if ( link->status > Link::LINK_CLOSED )
        {
            if ( link->fromNode->isClosed(q) || link->toNode->isClosed(-q) )
            {
                link->status = Link::TEMP_CLOSED;
                link->flow = ZERO_FLOW;
            }
        }

        //... write status change to message log

        if (oldStatus != link->status)
        {
            if ( reportTrials )
            {
                network->msgLog << endl << link->writeStatusChange(oldStatus);
            }
            result = true;
        }
    }
	//if ( result && reportTrials ) network->msgLog << endl;

    // --- look for status changes caused by pressure switch controls
    ////////  TO BE ADDED  ////////
    //changeCount += control_ApplyPressureControls();
    return result;
}

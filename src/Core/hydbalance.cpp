/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

//////////////////////////////////////////////
// Implementation of the HydBalance class.  //
//////////////////////////////////////////////

// TO DO:
// - compute and report system wide cumulative flow balance

#include "hydbalance.h"
#include "network.h"
#include "Elements/node.h"
#include "Elements/link.h"

#include <cmath>
#include <cstring>
using namespace std;

void   findNodeOutflows(double lamda, double dH[], double xQ[], Network* nw);
void   findLeakageFlows(double lamda, double dH[], double xQ[], Network* nw);
double findTotalFlowChange(double lamda, double dQ[], Network* nw);

//-----------------------------------------------------------------------------

//  Evaluate the error in satisfying the conservation of flow and energy
//  equations by an updated set of network heads and flows.
//

double  HydBalance::evaluate(
            double lamda,  // step size
            double dH[],   // change in nodal heads
            double dQ[],   // change in link flows
            double xQ[],   // nodal inflow minus outflow
            Network* nw)   // network being analyzed
{
    // ... initialize which elements have the maximum errors
    maxFlowErr = 0.0;
    maxHeadErr = 0.0;
    maxFlowChange = 0.0;
    maxHeadErrLink = -1;
    maxFlowErrNode = -1;
    maxFlowChangeLink = -1;

    // ... initialize nodal flow imbalances to 0

    int nodeCount = nw->count(Element::NODE);
    memset(&xQ[0], 0, nodeCount*sizeof(double));

    // ... find the error norm in satisfying conservation of energy
    //     (updating xQ with internal link flows)

    double norm = findHeadErrorNorm(lamda, dH, dQ, xQ, nw);

    // ... update xQ with external outflows

    findNodeOutflows(lamda, dH, xQ, nw);

    // ... add the error norm in satisfying conservation of flow

    norm += findFlowErrorNorm(xQ, nw);

    // ... evaluate the total relative flow change

    totalFlowChange = findTotalFlowChange(lamda, dQ, nw);

    // ... return the root mean square error

    return sqrt(norm);
}

//-----------------------------------------------------------------------------

//  Find the error norm in satisfying the head loss equation across each link.

double HydBalance::findHeadErrorNorm(
        double lamda, double dH[], double dQ[], double xQ[], Network* nw)
{
    double norm = 0.0;
    double count = 0.0;
    maxHeadErr = 0.0;
    maxFlowChange = 0.0;
    maxFlowChangeLink = 0;

    int linkCount = nw->count(Element::LINK);
    for (int i = 0; i < linkCount; i++)
    {
        // ... identify link's end nodes

        Link* link = nw->link(i);
        int n1 = link->fromNode->index;
        int n2 = link->toNode->index;

        // ... apply updated flow to end node flow balances

        double flowChange = lamda * dQ[i];
        double flow = link->flow + flowChange;
        xQ[n1] -= flow;
        xQ[n2] += flow;

        // ... update network's max. flow change

        double err = abs(flowChange);
        if ( err > maxFlowChange )
        {
            maxFlowChange = err;
            maxFlowChangeLink = i;
        }

        // ... compute head loss and its gradient (head loss is saved
        // ... to link->hLoss and its gradient to link->hGrad)
//*******************************************************************
        link->findHeadLoss(nw, flow);
//*******************************************************************

        // ... evaluate head loss error

        double h1 = link->fromNode->head + lamda * dH[n1];
        double h2 = link->toNode->head + lamda * dH[n2];
        if ( link->hGrad == 0.0 ) link->hLoss = h1 - h2;
        err = h1 - h2 - link->hLoss;
        if ( abs(err) > maxHeadErr )
        {
            maxHeadErr = abs(err);
            maxHeadErrLink = i;
        }

        // ... update sum of squared errors

        norm += err * err;
        count += 1.0;
    }

    // ... return sum of squared errors normalized by link count

    if ( count == 0.0 ) return 0;
    else return norm / count;

}

//-----------------------------------------------------------------------------

//  Find net external outflow at each network node.

void findNodeOutflows(double lamda, double dH[], double xQ[], Network* nw)
{
    // ... initialize node outflows and their gradients w.r.t. head

    for (Node* node : nw->nodes)
    {
        node->outflow = 0.0;
        node->qGrad = 0.0;
    }

    // ... find pipe leakage flows & assign them to node outflows

    if ( nw->leakageModel ) findLeakageFlows(lamda, dH, xQ, nw);

    // ... add emitter flows and demands to node outflows

    int nodeCount = nw->count(Element::NODE);
    for (int i = 0; i < nodeCount; i++)
    {
        Node* node = nw->node(i);
        double h = node->head + lamda * dH[i];
        double q = 0.0;
        double dqdh = 0.0;

        // ... for junctions, outflow depends on head

        if ( node->type() == Node::JUNCTION )
        {
            // ... contribution from emitter flow

            q = node->findEmitterFlow(h, dqdh);
            node->qGrad += dqdh;
            node->outflow += q;
            xQ[i] -= q;

            // ... contribution from demand flow

            // ... for fixed grade junction, demand is remaining flow excess
            if ( node->fixedGrade )
            {
                q = xQ[i];
                xQ[i] -= q;
            }

            // ... otherwise junction has pressure-dependent demand
            else
            {
                q = node->findActualDemand(nw, h, dqdh);
                node->qGrad += dqdh;
                xQ[i] -= q;
            }
            node->actualDemand = q;
            node->outflow += q;
        }

        // ... for tanks and reservoirs all flow excess becomes outflow

        else
        {
            node->outflow = xQ[i];
            xQ[i] = 0.0;
        }
    }
}

//-----------------------------------------------------------------------------

//  Find the error norm in satisfying flow continuity at each node.

double HydBalance::findFlowErrorNorm(double xQ[], Network* nw)
{
// Note: contributions to the nodal flow imbalance array xQ[] were
//       found previously from findHeadErrorNorm() and findNodeOutflows())

    double norm = 0.0;
    maxFlowErr = 0.0;

    int nodeCount = nw->count(Element::NODE);
    for (int i = 0; i < nodeCount; i++)
    {
        // ... update network's max. flow error

        if ( abs(xQ[i]) > maxFlowErr )
        {
            maxFlowErr = abs(xQ[i]);
            maxFlowErrNode = i;
        }

        // ... update sum of squared errors (flow imbalances)

        norm += xQ[i] * xQ[i];
    }

    // ... return sum of squared errors normalized by number of nodes

    return norm / nodeCount;
}

//-----------------------------------------------------------------------------

//  Assign the leakage flow along each network pipe to its end nodes.

void findLeakageFlows(double lamda, double dH[], double xQ[], Network* nw)
{
    double dqdh = 0.0;  // gradient of leakage outflow w.r.t. pressure head

    for (Link* link : nw->links)
    {
        // ... skip links that don't leak

        link->leakage = 0.0;
        dqdh = 0.0;
        if ( !link->canLeak() ) continue;

        // ... identify link's end nodes and their indexes

        Node* node1 = link->fromNode;
        Node* node2 = link->toNode;
        int n1 = node1->index;
        int n2 = node2->index;

        // ... no leakage if neither end node is not a junction

        bool canLeak1 = (node1->type() == Node::JUNCTION);
        bool canLeak2 = (node2->type() == Node::JUNCTION);
        if ( !canLeak1 && !canLeak2 ) continue;

        // ... find link's average pressure head

        double h1 = node1->head + lamda * dH[n1] - node1->elev;
        double h2 = node2->head + lamda * dH[n2] - node2->elev;
        double h = (h1 + h2) / 2.0;
        if ( h <= 0.0 ) continue;

        // ... find leakage and its gradient

        link->leakage = link->findLeakage(nw, h, dqdh);

        // ... split leakage flow between end nodes, unless one cannot
        //     support leakage or has negative pressure head

        double q = link->leakage / 2.0;
        if ( h1 * h2 <= 0.0 || canLeak1 * canLeak2 == 0 ) q = 2.0 * q;

        // ... add leakage to each node's outflow

        if ( h1 > 0.0 && canLeak1 )
        {
            node1->outflow += q;
            node1->qGrad += dqdh;
            xQ[n1] -= q;
        }
        if ( h2 > 0.0 && canLeak2 )
        {
            node2->outflow += q;
            node2->qGrad += dqdh;
            xQ[n2] -= q;
        }
    }
}

//-----------------------------------------------------------------------------

//  Find the sum of all link flow changes relative to the sum of all link flows.

double findTotalFlowChange(double lamda, double dQ[], Network* nw)
{
    double qSum = 0.0;
    double dqSum = 0.0;
    double dq;

    for ( int i = 0; i < nw->count(Element::LINK); i++ )
    {
        Link* link = nw->link(i);
        dq = lamda * dQ[i];
        dqSum += abs(dq);
        qSum += abs(link->flow + dq);
    }
    if ( qSum > 0.0 ) return dqSum / qSum;
    else return dqSum;
}

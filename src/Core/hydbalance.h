/* EPANET 3.1
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file hydbalance.h
//! \brief Describes the HydBalance class.

#ifndef HYDBALANCE_H_
#define HYDBALANCE_H_

class Network;

//! \class HydBalance
//! \brief Computes the degree to which a network solution is unbalanced.
//!
//! The HydBalance class determines the error in satisfying the head loss
//! equation across each link and the flow continuity equation at each node
//! of the network for an incremental change in nodal heads and link flows.

struct HydBalance
{
    double    maxFlowErr;         //!< max. flow error (cfs)
    double    maxHeadErr;         //!< max. head loss error (ft)
	double    h1ini;
	double    h1;
	double    h2ini;
	double    h2;
	double    phloss;
	double    gHn;
	double    previousMaxHeadErr; //!< previous max. head loss error (ft)
    double    maxFlowChange;      //!< max. flow change (cfs)
	double    previousMaxFlowChange;  //!< previous max. flow change (cfs)
    double    totalFlowChange;    //!< (summed flow changes) / (summed flows)

    int       maxHeadErrLink;     //!< link with max. head loss error
    int       maxFlowErrNode;     //!< node with max. flow error
    int       maxFlowChangeLink;  //!< link with max. flow change

    double    evaluate(
                  double lamda, double dH[], double dQ[], double xQ[], Network* nw, int currentTime, double tstep);
    double    findHeadErrorNorm(
		double lamda, double dH[], double dQ[], double xQ[], Network* nw, int currentTime, double tstep);
    double    findFlowErrorNorm(double xQ[], Network* nw);
};

#endif

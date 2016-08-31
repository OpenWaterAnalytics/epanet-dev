/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file ltdsolver.h
//! \brief Describes the LTDSolver class.

#ifndef LTDSOLVER_H_
#define LTDSOLVER_H_

#include "Solvers/qualsolver.h"
#include "Utilities/segpool.h"
#include <vector>

class Network;

//! \class LTDSolver
//! \brief A water quality solver based on the Lagrangian Time Driven method.

class LTDSolver : public QualSolver
{
  public:

    LTDSolver(Network* nw);
    ~LTDSolver();

    void init();
    void reverseFlow(int k);
    int  solve(int* sortedLinks, int timeStep);

  private:
	int                    nodeCount;        // number of nodes
	int                    linkCount;        // number of links
	double                 cTol;             // quality tolerance (mass/ft3)
	double                 tstep;            // time step (sec)

	std::vector<double>    volIn;            // volume inflow to each node
	std::vector<double>    massIn;           // mass inflow to each node
	std::vector<Segment *> firstSegment;     // ptr. to first segment in each link
	std::vector<Segment *> lastSegment;      // ptr. to last segment in each link
	SegPool                segPool;          // pool of pipe segment objects

	void   react();
	void   release(int k);
	void   transport(int k);
	void   updateNodeQuality();
	void   updateLinkQuality();
	double findStoredMass();
	void   updateMassBalance();
    void   addSegment(int k, double v, double c);

};

#endif

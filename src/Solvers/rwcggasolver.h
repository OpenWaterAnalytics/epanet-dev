/* EPANET 3.1
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file rwcggasolver.h
//! \brief Describes the RWCGGASolver class.

#ifndef RWCGGASOLVER_H_
#define RWCGGASOLVER_H_

#include "Solvers/hydsolver.h"
#include "Core/hydbalance.h"

#include <vector>

class HydSolver;

//! \class RWCGGASolver
//! \brief A hydraulic solver based on RWC Global Gradient Algorithm.

class RWCGGASolver : public HydSolver
{
  public:

    RWCGGASolver(Network* nw, MatrixSolver* ms);
    ~RWCGGASolver();
    int solve(double tstep, int& trials, int currentTime);
	double     tstep;             // time step (sec)

  private:

    int        nodeCount;         // number of network nodes
    int        linkCount;         // number of network links
    int        hLossEvalCount;    // number of head loss evaluations
    int        stepSizing;        // Newton step sizing method
	int        lambdaNumber;

    int        trialsLimit;       // limit on number of trials
    bool       reportTrials;      // report summary of each trial
    double     headErrLimit;      // allowable head error (ft)
    double     flowErrLimit;      // allowable flow error (cfs)
    double     flowChangeLimit;   // allowable flow change (cfs)
    double     flowRatioLimit;    // allowable total flow change / total flow
    
    double     theta;             // time weighting constant
	double     kappa;             // temporal discretization parameter
	double     epsilon;           // convergence parameter
	double     dhmaxpast;
	double     maxHeadErrPast;
	double     maxHeadError;
	double     minErrorNorm;
	double     dl;

    double     errorNorm;         // solution error norm
    double     oldErrorNorm;      // previous error norm
    HydBalance hydBalance;        // hydraulic balance results

    std::vector<double> dH;       // head change at each node (ft)
    std::vector<double> dQ;       // flow change in each link (cfs)
    std::vector<double> xQ;       // node flow imbalances (cfs)
	std::vector<double> Lambda;

    // Functions that assemble linear equation coefficients
    void   setFixedGradeNodes();
    void   setMatrixCoeffs();
    void   setLinkCoeffs();
    void   setNodeCoeffs();
    void   setValveCoeffs();
	void   setInertialTerm();


    // Functions that update the hydraulic solution
    int    findHeadChanges();
    void   findFlowChanges();
    double findStepSize(int trials, int currentTime);
    void   updateSolution(double lamda);

    // Functions that check for convergence
    void   setConvergenceLimits();
    double findErrorNorm(double lamda, int currentTime, double tstep);
    bool   hasConverged();
    bool   linksChangedStatus();
    void   reportTrial(int trials, double lamda);
};

#endif

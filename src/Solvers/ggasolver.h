/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file ggasolver.h
//! \brief Describes the GGASolver class.

#ifndef GGASOLVER_H_
#define GGASOLVER_H_

#include "Solvers/hydsolver.h"
#include "Core/hydbalance.h"

#include <vector>

class HydSolver;

//! \class GGASolver
//! \brief A hydraulic solver based on Todini's Global Gradient Algorithm.

class GGASolver : public HydSolver
{
  public:

    GGASolver(Network* nw, MatrixSolver* ms);
    ~GGASolver();
    int solve(double tstep, int& trials);

  private:

    int        nodeCount;         // number of network nodes
    int        linkCount;         // number of network links
    int        hLossEvalCount;    // number of head loss evaluations
    int        stepSizing;        // Newton step sizing method

    int        trialsLimit;       // limit on number of trials
    bool       reportTrials;      // report summary of each trial
    double     headErrLimit;      // allowable head error (ft)
    double     flowErrLimit;      // allowable flow error (cfs)
    double     flowChangeLimit;   // allowable flow change (cfs)
    double     flowRatioLimit;    // allowable total flow change / total flow
    double     tstep;             // time step (sec)
    double     theta;             // time weighting constant

    double     errorNorm;         // solution error norm
    double     oldErrorNorm;      // previous error norm
    HydBalance hydBalance;        // hydraulic balance results

    std::vector<double> dH;       // head change at each node (ft)
    std::vector<double> dQ;       // flow change in each link (cfs)
    std::vector<double> xQ;       // node flow imbalances (cfs)

    // Functions that assemble linear equation coefficients
    void   setFixedGradeNodes();
    void   setMatrixCoeffs();
    void   setLinkCoeffs();
    void   setNodeCoeffs();
    void   setValveCoeffs();

    // Functions that update the hydraulic solution
    int    findHeadChanges();
    void   findFlowChanges();
    double findStepSize(int trials);
    void   updateSolution(double lamda);

    // Functions that check for convergence
    void   setConvergenceLimits();
    double findErrorNorm(double lamda);
    bool   hasConverged();
    bool   linksChangedStatus();
    void   reportTrial(int trials, double lamda);
};

#endif

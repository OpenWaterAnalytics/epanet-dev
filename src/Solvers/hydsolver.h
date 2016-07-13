/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file hydsolver.h
//! \brief Describes the HydSolver class.

#ifndef HYDSOLVER_H_
#define HYDSOLVER_H_

#include <string>

class Network;
class MatrixSolver;

//! \class HydSolver
//! \brief Interface for an equilibrium network hydraulic solver.
//!
//! This is an abstract class that defines an interface for a
//! specific algorithm used for solving pipe network hydraulics at a
//! given instance in time.

class HydSolver
{
  public:

    enum StatusCode {
        SUCCESSFUL,
        FAILED_NO_CONVERGENCE,
        FAILED_ILL_CONDITIONED
    };

    HydSolver(Network* nw, MatrixSolver* ms);
    virtual ~HydSolver();
    static  HydSolver* factory(const std::string name, Network* nw, MatrixSolver* ms);
    virtual int solve(double tstep, int& trials) = 0;

  protected:

    Network*       network;
    MatrixSolver*  matrixSolver;

};

#endif

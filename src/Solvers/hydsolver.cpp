/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "hydsolver.h"

// Include header files for the different hydraulic solvers here.
#include "ggasolver.h"

using namespace std;

HydSolver::HydSolver(Network* nw, MatrixSolver* ms) :
    network(nw), matrixSolver(ms)
{}

HydSolver::~HydSolver() {}

HydSolver* HydSolver::factory(const string name, Network* nw, MatrixSolver* ms)
{
    if (name == "GGA") return new GGASolver(nw, ms);
    return nullptr;
}

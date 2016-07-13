/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "qualsolver.h"

// Include headers for the different quality solvers here
#include "ltdsolver.h"

using namespace std;

QualSolver::QualSolver(Network* nw) : network(nw) {}

QualSolver::~QualSolver() {}

QualSolver* QualSolver::factory(const string name, Network* nw)
{
    if ( name == "LTD" ) return new LTDSolver(nw);
    return nullptr;
}

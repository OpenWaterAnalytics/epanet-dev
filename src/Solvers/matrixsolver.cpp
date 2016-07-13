/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "matrixsolver.h"

// Include headers for the different matrix solvers here
#include "sparspaksolver.h"
//#include "cholmodsolver.h"

using namespace std;

MatrixSolver::MatrixSolver() {}

MatrixSolver::~MatrixSolver() {}

MatrixSolver* MatrixSolver::factory(const string name, ostream& logger)
{
    //if (name == "CHOLMOD") return new CholmodSolver();
    if (name == "SPARSPAK") return new SparspakSolver(logger);
    return nullptr;
}

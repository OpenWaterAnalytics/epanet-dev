/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file qualsolver.h
//! \brief Describes the QualSolver class.

#ifndef QUALSOLVER_H_
#define QUALSOLVER_H_

#include "Core/qualbalance.h"
#include <string>

class Network;
class Link;

//! \class QualSolver
//! \brief Abstract class from which a specific water quality solver is derived.

class QualSolver
{
  public:

    // Constructor/Destructor
    QualSolver(Network* nw);
    virtual ~QualSolver();

    // Factory method
    static  QualSolver* factory(const std::string model, Network* nw);

    // Public Methods
    virtual void   init() { }
    virtual void   reverseFlow(int linkIndex) { }
    virtual int    solve(int* sortedLinks, int timeStep) = 0;

  protected:
    Network*     network;
};

#endif

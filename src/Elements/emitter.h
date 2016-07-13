/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

//! \file emitter.h
//! \brief Describes the Emitter class.

#ifndef EMITTER_H_
#define EMITTER_H_

//! \class Emitter
//! \brief Models an unlimited pressure-dependent rate of water outflow
//!        at a Junction node.
//!

class Network;
class Pattern;
class Junction;

class Emitter
{
  public:

    // Constructor/Destructor
    Emitter();
    ~Emitter();

    // Static factory method to add or edit an emitter
    static bool addEmitter(Junction* junc, double c, double e, Pattern* p);

    // Converts emitter properties to internal units
    void        convertUnits(Network* network);

    // Finds the emitter's outflow rate and its derivative given the pressure head
    double      findFlowRate(double h, double& dqdh);

    // Properties
    double      flowCoeff;     // flow = flowCoeff*(head^expon)
    double      expon;
    Pattern*    timePattern;   // pattern for time varying flowCoeff
};

#endif

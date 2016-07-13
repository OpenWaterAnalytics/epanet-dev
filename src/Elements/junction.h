/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

//! \file junction.h
//! \brief Describes the Junction class.

#ifndef JUNCTION_H_
#define JUNCTION_H_

#include "Elements/node.h"
#include "Elements/demand.h"

#include <string>
#include <list>

class Network;
class Emitter;

//! \class Junction
//! \brief A variable head Node with no storage volume.

class Junction: public Node
{
  public:

    Junction(std::string name_);
    ~Junction();

    int    type() { return Node::JUNCTION; }
    void   convertUnits(Network* nw);
    void   initialize(Network* nw);
    void   findFullDemand(double multiplier, double patternFactor);
    double findActualDemand(Network* nw, double h, double& dqdh);
    double findEmitterFlow(double h, double& dqdh);
    bool   isPressureDeficient(Network* nw);
    bool   hasEmitter() { return emitter != nullptr; }

    Demand            primaryDemand;   //!< primary demand
    std::list<Demand> demands;         //!< collection of additional demands
    double            pMin;            //!< minimum pressure head to have demand (ft)
    double            pFull;           //!< pressure head required for full demand (ft)
    Emitter*          emitter;         //!< emitter object
};

#endif

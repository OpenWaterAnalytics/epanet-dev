/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

//! \file node.h
//! \brief Describes the Node class.

#ifndef NODE_H_
#define NODE_H_

#include "Elements/element.h"

#include <string>

class Network;
class Emitter;
class QualSource;
class MemPool;

//! \class Node
//! \brief A connection point between links in a network.
//!
//! A Node is an abstract class that represents the end connection points
//! between links in a pipe network. Specific concrete sub-classes of Node
//! include Junction, Tank, and Reservoir.

class Node: public Element
{
  public:

    enum NodeType  {JUNCTION, TANK, RESERVOIR};

    Node(std::string name_);
    virtual ~Node();

    static  Node*  factory(int type_, std::string name_, MemPool* memPool);

    virtual int    type() = 0;
    virtual void   convertUnits(Network* nw) = 0;
    virtual void   initialize(Network* nw);

    // Overridden for Junction nodes
    virtual void   findFullDemand(double multiplier, double patternFactor) { }
    virtual double findActualDemand(Network* nw, double h, double& dqdh) { return 0; }
    virtual double findEmitterFlow(double h, double& dqdh) { return 0; }
    virtual void   setFixedGrade() { fixedGrade = false; }
    virtual bool   isPressureDeficient(Network* nw) { return false; }
    virtual bool   hasEmitter() { return false; }

    // Overridden for Tank nodes
    virtual void   validate(Network* nw) { }
    virtual bool   isReactive() { return false; }
    virtual bool   isFull() { return false; }
    virtual bool   isEmpty() { return false; }
    virtual bool   isClosed(double flow) { return false; }
    virtual double getVolume() { return 0.0; }

    // Input Parameters
    bool           rptFlag;       //!< true if results are reported
    double         elev;          //!< elevation (ft)
    double         xCoord;        //!< X-coordinate
    double         yCoord;        //!< Y-coordinate
    double         initQual;      //!< initial water quality concen.
    QualSource*    qualSource;    //!< water quality source information

    // Computed Variables
    bool           fixedGrade;    //!< fixed grade status
    double         head;          //!< hydraulic head (ft)
    double         qGrad;         //!< gradient of outflow w.r.t. head (cfs/ft)
    double         fullDemand;    //!< full demand required (cfs)
    double         actualDemand;  //!< actual demand delivered (cfs)
    double         outflow;       //!< demand + emitter + leakage flow (cfs)
    double         quality;       //!< water quality concen. (mass/ft3)
};

#endif

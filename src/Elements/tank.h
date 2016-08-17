/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file tank.h
//! \brief Describes the Tank class.

#ifndef TANK_H_
#define TANK_H_

#include "Elements/node.h"
#include "Models/tankmixmodel.h"

#include <string>

class Curve;

//! \class Tank
//! \brief A fixed head Node with storage volume.
//!
//! The fixed head for the tank varies from one time period to the next
//! depending on the filling or withdrawal rate.

class Tank: public Node
{
  public:

    // Constructor/Destructor
    Tank(std::string name);
    //~Tank() {}

    // Overridden virtual methods
    int    type() { return Node::TANK; }
    void   validate(Network* nw);
    void   convertUnits(Network* nw);
    void   initialize(Network* nw);
    bool   isReactive() { return bulkCoeff != 0.0; }
    bool   isFull()     { return head >= maxHead; }
    bool   isEmpty()    { return head <= minHead; }
    bool   isClosed(double flow);

    // Tank-specific methods
    double getVolume() { return volume; }
    double findVolume(double aHead);
    double findHead(double aVolume);
    void   setFixedGrade();
    void   updateVolume(int tstep);
    void   updateArea();
    int    timeToVolume(double aVolume);

    // Properties
    double initHead;               //!< initial water elevation (ft)
    double minHead;                //!< minimum water elevation (ft)
    double maxHead;                //!< maximum water elevation (ft)
    double diameter;               //!< nominal diameter (ft)
    double minVolume;              //!< minimum volume (ft3)
    double bulkCoeff;              //!< water quality reaction coeff. (per day)
    Curve* volCurve;               //!< volume v. water depth curve
    TankMixModel mixingModel;      //!< mixing model used

    double maxVolume;              //!< maximum volume (ft3)
    double volume;                 //!< current volume in tank (ft3)
    double area;                   //!< current surface area of tank (ft2)
    double ucfLength;              //!< units conversion factor for length
    double pastHead;               //!< water elev. in previous time period (ft)
    double pastVolume;             //!< volume in previous time period (ft3)
    double pastOutflow;            //!< outflow in previous time period (cfs)
};

#endif

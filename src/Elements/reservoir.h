/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file reservoir.h
//! \brief Describes the Reservoir class.

#ifndef RESERVOIR_H_
#define RESERVOIR_H_

#include "Elements/node.h"

#include <string>

class Network;
class Pattern;

//! \class Reservoir
//! \brief A fixed head Node with no storage volume.
//!
//! \note The reservoir's fixed head can be made to vary over time by
//!       specifying a time pattern.

class Reservoir: public Node
{
  public:

    // Constructor/Destructor
    Reservoir(std::string name_);
    ~Reservoir();

    // Methods
    int      type() { return Node::RESERVOIR; }
    void     convertUnits(Network* nw);
    void     setFixedGrade();

    // Properties
    Pattern* headPattern;    //!< time pattern for reservoir's head
};

#endif

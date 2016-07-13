/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file control.h
//! \brief Describes the Control class.

#ifndef CONTROL_H_
#define CONTROL_H_

#include "Elements/element.h"

#include <string>
#include <ostream>

class Node;
class Tank;
class Link;
class Network;

//! \class Control
//! \brief A class that controls pumps and valves based on a single condition.

class Control: public Element
{
  public:
    enum ControlType {TANK_LEVEL, PRESSURE_LEVEL, ELAPSED_TIME, TIME_OF_DAY,
                      RULE_BASED};
    enum StatusType  {CLOSED_STATUS, OPEN_STATUS, NO_STATUS};
    enum LevelType   {LOW_LEVEL, HI_LEVEL, NO_LEVEL};

    // Constructor/Destructor
    Control(int type_, std::string name_);
    ~Control();

    // Applies all pressure controls to the pipe network
    static  void     applyPressureControls(Network* network);

    // Sets the properties of a control
    void    setProperties(
                int    controlType,
                Link*  controlLink,
                int    linkStatus,
                double linkSetting,
                Node*  controlNode,
                double nodeSetting,
                int    controlLevelType,
                int    timeSetting);

    // Produces a string representation of the control
    std::string toStr(Network* network);

    // Converts the control's properties to internal units
    void    convertUnits(Network* network);

    // Returns the control's type (see ControlType enum)
    int     getType()
            { return type; }

    // Finds the time until the control is next activated
    int    timeToActivate(Network* network, int t, int tod);

    // Checks if the control's conditions are met
    void    apply(Network* network, int t, int tod);

  private:
    int         type;                  //!< type of control
    Link*       link;                  //!< link being controlled
    int         status;                //!< open/closed setting for link
    double      setting;               //!< speed or valve setting for link
    Node*       node;                  //!< node that triggers control action
    double      head;                  //!< head that triggers control action
    double      volume;                //!< volume corresponding to head trigger
    LevelType   levelType;             //!< type of node head trigger
    int         time;                  //!< time (sec) that triggers control

    // Activates the control's action
    bool        activate(bool makeChange, std::ostream& msgLog);

};

#endif

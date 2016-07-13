/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file qualengine.h
//! \brief Describes the QualEngine class.

#ifndef QUALENGINE_H_
#define QUALENGINE_H_

#include <vector>

class Network;
class QualSolver;
//class JuncMixer;
//class TankMixer;

//! \class QualEngine
//! \brief Simulates extended period water quality in a network.
//!
//! The QualEngine class carries out an extended period water quality simulation
//! on a pipe network, calling on its QualSolver object to solve the reaction,
//! transport and mixing equations at each time step.

class QualEngine
{
  public:

    // Constructor/Destructor

    QualEngine();
    ~QualEngine();

    // Public Methods

    void   open(Network* nw);
    void   init();
    void   solve(int tstep);
    void   close();

private:

    // Engine state

    enum EngineState {CLOSED, OPENED, INITIALIZED};
    EngineState engineState;

    // Engine components

    Network*    network;            //!< network being analyzed
    QualSolver* qualSolver;         //!< single time step water quality solver

    // Engine properties

    int         nodeCount;          //!< number of network nodes
    int         linkCount;          //!< number of network links
    int         qualTime;           //!< current simulation time (sec)
    int         qualStep;           //!< hydraulic time step (sec)
    std::vector<int>  sortedLinks;      //!< topologically sorted links
    std::vector<char> flowDirection;    //!< direction (+/-) of link flow

    // Simulation sub-tasks

    bool        flowDirectionsChanged();
    void        setFlowDirections();
    void        sortLinks();
    void        setSourceQuality();
};

#endif

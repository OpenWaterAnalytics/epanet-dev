/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

//! \file hydengine.h
//! \brief Describes the HydEngine class.

#ifndef HYDENGINE_H_
#define HYDENGINE_H_

#include <string>

class Network;
class HydSolver;
class MatrixSolver;

//! \class HydEngine
//! \brief Simulates extended period hydraulics.
//!
//! The HydEngine class carries out an extended period hydraulic simulation on
//! a pipe network, calling on its HydSolver object to solve the conservation of
//! mass and energy equations at each time step.

class HydEngine
{
  public:

    // Constructor/Destructor

    HydEngine();
    ~HydEngine();

    // Public Methods

    void   open(Network* nw);
    void   init(bool initFlows);
    int    solve(int* t);
    void   advance(int* tstep);
    void   close();

    int    getElapsedTime() { return currentTime; }
    double getPeakKwatts()  { return peakKwatts;  }

  private:

    // Engine state

    enum EngineState {CLOSED, OPENED, INITIALIZED};
    EngineState engineState;

    // Engine components

    Network*       network;            //!< network being analyzed
    HydSolver*     hydSolver;          //!< steady state hydraulic solver
    MatrixSolver*  matrixSolver;       //!< sparse matrix solver
//    HydFile*       hydFile;            //!< hydraulics file accessor

    // Engine properties

    bool           saveToFile;         //!< true if results saved to file
    bool           halted;             //!< true if simulation has been halted
    int            startTime;          //!< starting time of day (sec)
    int            rptTime;            //!< current reporting time (sec)
    int            hydStep;            //!< hydraulic time step (sec)
    int            currentTime;        //!< current simulation time (sec)
    int            timeOfDay;          //!< current time of day (sec)
    double         peakKwatts;         //!< peak energy usage (kwatts)
    std::string    timeStepReason;     //!< reason for taking next time step

    // Simulation sub-tasks

    void           initMatrixSolver();

    int            getTimeStep();
    int            timeToPatternChange(int tstep);
    int            timeToActivateControl(int tstep);
    int            timeToCloseTank(int tstep);

    void           updateCurrentConditions();
    void           updateTanks();
    void           updatePatterns();
    void           updateEnergyUsage();

    bool           isPressureDeficient();
    int            resolvePressureDeficiency(int& trials);
    void           reportDiagnostics(int statusCode, int trials);
};

#endif

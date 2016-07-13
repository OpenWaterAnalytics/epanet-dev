/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file qualbalance.h
//! \brief Describes the QualBlanace class.

#ifndef QUALBALANCE_H_
#define QUALBALANCE_H_

#include <ostream>

//! \class QualBalance
//! \brief Computes a water quality mass balance across the pipe network.
//!
//! The QualBalance structure updates the total inflow, outflow, storage and
//! losses of a water quality constituent at each time step of the simulation
//! and computes its overall mass balance at the end of the simulation.

class Network;

struct QualBalance
{
    double    initMass;
    double    inflowMass;
    double    outflowMass;
    double    reactedMass;
    double    storedMass;

    void      init(const double initMassStored);
    void      updateInflow(const double massIn);
    void      updateOutflow(const double massOut);
    void      updateReacted(const double massReacted);
    void      updateStored(const double massStored);
    void      writeBalance(std::ostream& msgLog);
};

//-----------------------------------------------------------------------------
//    Inline Functions
//-----------------------------------------------------------------------------
inline void QualBalance::updateInflow(const double massIn)
            { inflowMass += massIn; }

inline void QualBalance::updateOutflow(const double massOut)
            { outflowMass += massOut; }

inline void QualBalance::updateReacted(const double massReacted)
            { reactedMass += massReacted; }

inline void QualBalance::updateStored(const double massStored)
            { storedMass = massStored; }

#endif // QUALBALANCE_H_

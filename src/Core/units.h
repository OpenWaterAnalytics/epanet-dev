/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file units.h
//! \brief Describes the Units class.

#ifndef UNITS_H_
#define UNITS_H_

#include "Core/options.h"

#include <string>

//! \class Units
//! \brief Defines units conversion factors for network quantities.

class Units
{
  public:

    enum Quantity {
        DIAMETER,
        LENGTH,
        VOLUME,
        POWER,
        PRESSURE,
        FLOW,
        VELOCITY,
        HEADLOSS,
        CONCEN,
        MAX_QUANTITIES
    };

    Units();
    ~Units() {}

    void        setUnits(Options& options);
    double      factor(Quantity quantity);
    std::string name(Quantity quantity);

  private:

    double      factors[MAX_QUANTITIES];  //!< array of unit conversion factors
    std::string names[MAX_QUANTITIES];    //!< array of unit names

    int    setFlowFactor(const int flowUnits);
    void   setPressureFactor(const int unitSystem, Options& options);
    void   setOtherFactors(int unitSystem);
};

//-----------------------------------------------------------------------------
//    Inline Functions
//-----------------------------------------------------------------------------
inline double Units::factor(Quantity quantity) {return factors[quantity];}

inline std::string Units::name(Quantity quantity) {return names[quantity];}

#endif

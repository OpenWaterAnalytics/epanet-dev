/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file pumpenergy.h
//! \brief Describes the PumpEnergy class.

#ifndef PUMPENERGY_H_
#define PUMPENERGY_H_

class Pump;
class Network;

//! \class PumpEnergy
//! \brief Accumulates energy usage metrics for a pump.

class PumpEnergy
{
  public:

    // Constructor
    PumpEnergy();

    // Methods
    void   init();
    double updateEnergyUsage(Pump* pump, Network* network, int dt);

    // Computed Properties
    double hrsOnLine;        //!< hours pump is online
    double efficiency;       //!< total time wtd. efficiency
    double kwHrsPerCFS;      //!< total kw-hrs per cfs of flow
    double kwHrs;            //!< total kw-hrs consumed
    double maxKwatts;        //!< max. kw consumed
    double totalCost;        //!< total pumping cost

  private:

    double findCostFactor(Pump* pump, Network* network);
    double findEfficiency(Pump* pump, Network* network);
};

#endif

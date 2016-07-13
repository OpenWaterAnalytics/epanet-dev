/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file pump.h
//! \brief Describes the Pump class.

#ifndef PUMP_H_
#define PUMP_H_

#include "Elements/link.h"
#include "Elements/pumpcurve.h"
#include "Models/pumpenergy.h"

#include <string>

class Network;
class Pattern;
class Curve;

//! \class Pump
//! \brief A Link that raises the head of water flowing through it.

class Pump: public Link
{
  public:

    // Constructor/Destructor

    Pump(std::string name_);
    ~Pump();

    // Methods

    int         type() { return Link::PUMP; }
    std::string typeStr() { return "Pump"; }
    void        convertUnits(Network* nw);
    void        validate(Network* nw);

    void        setInitFlow();
    void        setInitStatus(int s);
    void        setInitSetting(double s);
    double      getSetting(Network* nw) { return speed; }

    bool        isHpPump() { return pumpCurve.isConstHP(); }

    void        findHeadLoss(Network* nw, double q);
    double      updateEnergyUsage(Network* nw, int dt);

    bool        changeStatus(int s, bool makeChange,
                             const std::string reason,
                             std::ostream& msgLog);
    bool        changeSetting(double s, bool makeChange,
                              const std::string reason,
                              std::ostream& msgLog);
    void        validateStatus(Network* nw, double qTol);
    void        applyControlPattern(std::ostream& msgLog);

    // Properties

    PumpCurve  pumpCurve;      //!< pump's head v. flow relation
    double     speed;          //!< relative pump speed
    Pattern*   speedPattern;   //!< speed time pattern
    PumpEnergy pumpEnergy;     //!< pump's energy usage
    Curve*     efficCurve;     //!< efficiency. v. flow curve
    Pattern*   costPattern;    //!< energy cost pattern
    double     costPerKwh;     //!< unit energy cost (cost/kwh)
 };

#endif

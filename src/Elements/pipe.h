/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file pipe.h
//! \brief Describes the Pipe class.

#ifndef PIPE_H_
#define PIPE_H_

#include "Elements/link.h"

class Network;

//! \class Pipe
//! \brief A circular conduit Link through which water flows.

class Pipe: public Link
{
  public:

    // Constructor/Destructor

    Pipe(std::string name);
    ~Pipe();

    // Methods

    int         type() { return Link::PIPE; }
    std::string typeStr() { return "Pipe"; }
    void        convertUnits(Network* nw);
    bool        isReactive();
    void        setInitFlow();
    void        setInitStatus(int s);
    void        setInitSetting(double s);
    void        setResistance(Network* nw);

    double      getRe(const double q, const double viscos);
    double      getResistance() {return resistance;}
    double      getVelocity();
    double      getUnitHeadLoss();
    double      getSetting(Network* nw) { return roughness; }
    double      getVolume() { return 0.785398 * length * diameter * diameter; }

    void        findHeadLoss(Network* nw, double q);
    bool        canLeak() { return leakCoeff1 > 0.0; }
    double      findLeakage(Network* nw, double h, double& dqdh);
    bool        changeStatus(int s, bool makeChange,
                            const std::string reason,
                            std::ostream& msgLog);
    void        validateStatus(Network* nw, double qTol);

    // Properties

    bool   hasCheckValve;    //!< true if pipe has a check valve
    double length;           //!< pipe length (ft)
    double roughness;        //!< roughness parameter (units depend on head loss model)
    double resistance;       //!< resistance factor (units depend head loss model)
    double lossFactor;       //!< minor loss factor (ft/cfs^2)
    double leakCoeff1;       //!< leakage coefficient (user units)
    double leakCoeff2;       //!< leakage coefficient (user units)
    double bulkCoeff;        //!< bulk reaction coefficient (mass^n/sec)
    double wallCoeff;        //!< wall reaction coefficient (mass^n/sec)
    double massTransCoeff;   //!< mass transfer coefficient (mass^n/sec)
 };

#endif

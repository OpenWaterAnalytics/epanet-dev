/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file valve.h
//! \brief Describes the Valve class.

#ifndef VALVE_H_
#define VALVE_H_

#include "Elements/link.h"

#include <string>

class Network;

//! \class Valve
//! \brief A Link that controls flow or pressure.
//! \note Isolation (or shutoff) valves can be modeled by setting a
//!       pipe's Status property to OPEN or CLOSED.

class Valve: public Link
{
  public:

    enum ValveType {
        PRV,               //!< pressure reducing valve
        PSV,               //!< pressure sustaining valve
        FCV,               //!< flow control valve
        TCV,               //!< throttle control valve
        PBV,               //!< pressure breaker valve
        GPV                //!< general purpose valve
    };
    static const char* ValveTypeWords[];

    // Constructor/Destructor
    Valve(std::string name_);
    ~Valve();

    // Methods
    int         type();
    std::string typeStr();
    void        convertUnits(Network* nw);
    double      convertSetting(Network* nw, double s);

    void        setInitFlow();
    void        setInitStatus(int s);
    void        setInitSetting(double s);
    void        initialize(bool initFlow);

    bool        isPRV();
    bool        isPSV();

    void        findHeadLoss(Network* nw, double q);
    void        updateStatus(double q, double h1, double h2);
    bool        changeStatus(int newStatus,
                             bool makeChange,
                             const std::string reason,
                             std::ostream& msgLog);
    bool        changeSetting(double newSetting,
                              bool makeChange,
                              const std::string reason,
                              std::ostream& msgLog);
    void        validateStatus(Network* nw, double qTol);

    double      getVelocity();
    double      getRe(const double q, const double viscos);
    double      getSetting(Network* nw);

    // Properties
    ValveType   valveType;      //!< valve type
    double      lossFactor;     //!< minor loss factor

  protected:
    void        findOpenHeadLoss(double q);
    void        findPbvHeadLoss(double q);
    void        findTcvHeadLoss(double q);
    void        findGpvHeadLoss(Network* nw, double q);
    void        findFcvHeadLoss(double q);
    int         updatePrvStatus(double q, double h1, double h2);
    int         updatePsvStatus(double q, double h1, double h2);

    bool        hasFixedStatus;   //!< true if Open/Closed status is fixed
    double      elev;             //!< elevation of PRV/PSV valve
};

//-----------------------------------------------------------------------------
//    Inline Functions
//-----------------------------------------------------------------------------

inline
int  Valve::type() { return Link::VALVE; }

inline
bool Valve::isPRV() { return valveType == PRV; }

inline
bool Valve::isPSV() { return valveType == PSV; }

#endif

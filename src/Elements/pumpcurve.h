/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file pumpcurve.h
//! \brief Describes the PumpCurve class.

#ifndef PUMPCURVE_H_
#define PUMPCURVE_H_

class Network;
class Curve;

//! \class PumpCurve
//! \brief Describes how head varies with flow for a Pump link.

class PumpCurve
{
  public:

    enum PumpCurveType {
        NO_CURVE,              //!< no curve assigned
        CONST_HP,              //!< constant horsepower curve
        POWER_FUNC,            //!< power function curve
        CUSTOM                 //!< user-defined custom curve
    };

    // Constructor/Destructor
    PumpCurve();
    ~PumpCurve();

    // Methods
    void   setCurve(Curve* c);
    int    setupCurve(Network* network);
    void   findHeadLoss(
               double speed, double flow, double& headLoss, double& gradient);
    bool   isConstHP() {return curveType == CONST_HP;}

    // Properties
    int    curveType;      //!< type of pump curve
    Curve* curve;          //!< curve with head v. flow data
    double horsepower;     //!< pump's horsepower
    double qInit;          //!< initial flow (cfs)
    double qMax;           //!< maximum flow (cfs)
    double hMax;           //!< maximum head (ft)

  private:

    double h0;             //!< shutoff head (ft)
    double r;              //!< flow coefficient for power function curve
    double n;              //!< flow exponent for power function curve
    double qUcf;           //!< flow units conversion factor
    double hUcf;           //!< head units conversion factor

    void   setupConstHpCurve();
    int    setupPowerFuncCurve();
    int    setupCustomCurve();
    void   constHpHeadLoss(
               double speed, double flow, double& headLoss, double& gradient);
    void   powerFuncHeadLoss(
               double speed, double flow, double& headLoss, double& gradient);
    void   customCurveHeadLoss(
               double speed, double flow, double& headLoss, double& gradient);
};

#endif

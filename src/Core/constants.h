/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file constants.h
//! \brief Constants used throughout EPANET.

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

const int    VERSION = 30000;          //!< current version
const int    MAGICNUMBER = 1236385461; //!< magic number
const double MISSING = -999999999.9;   //!< missing value

const double PI = 3.141592654;         //!< value of PI
const double GRAVITY = 32.2;           //!< gravitational constant (ft/sec/sec)
const double VISCOSITY = 1.1E-5;       //!< kinematic viscosity of water @ 20 deg C (sq ft/sec)
const double DIFFUSIVITY = 1.3E-8;     //!< molecular diffusivity of chlorine @ 20 deg C (sq ft/sec)

// Conversion factors for US flow units
const double GPMperCFS =  448.831;
const double MGDperCFS =  0.64632;
const double IMGDperCFS = 0.5382;
const double AFDperCFS =  1.9837;

// Conversion factors for SI flow units
const double LPSperCFS =  28.317;
const double LPMperCFS =  1699.0;
const double CMHperCFS =  101.94;
const double CMDperCFS =  2446.6;
const double MLDperCFS =  2.4466;

// Conversion factors for SI quantities
const double LperFT3   =  28.317;
const double M3perFT3  =  0.028317;
const double MperFT    =  0.3048;
const double KPAperPSI =  6.895;
const double KWperHP   =  0.7457;

// Miscellaneous factors
const double SECperDAY =  86400.0;
const double PSIperFT  =  0.4333;
const double FT3perL   =  0.0353145;

const double MIN_GRADIENT    = 1.0e-6; //!< minimum head loss gradient
const double HIGH_RESISTANCE = 1.0e8;  //!< infinite flow resistance
const double HEAD_EPSILON    = 1.0e-6; //!< negligible head value (ft)
const double ZERO_FLOW       = 1.0e-6; //!< flow in closed link (cfs)

#endif

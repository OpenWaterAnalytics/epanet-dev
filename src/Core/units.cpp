/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//////////////////////////////////////////
//  Implementation of the Units class.  //
//////////////////////////////////////////

#include "units.h"
#include "constants.h"
using namespace std;

//-----------------------------------------------------------------------------

//  Constructor

Units::Units()
{
    // ... conversion factors for default US units
    factors[FLOW]     = GPMperCFS;
    names[FLOW]       = "gpm";
    factors[PRESSURE] = PSIperFT;
    names[PRESSURE]   = "psi";
    factors[CONCEN]   = FT3perL;
    names[CONCEN]     = "mg/L";
    setOtherFactors(Options::US);
}

//-----------------------------------------------------------------------------

//  Set unit conversion factors

void Units::setUnits(Options &options)
{
    // ... flow units determines flow conversion factor and units system

    int unitSystem = setFlowFactor(options.flowUnits());
    options.setOption(Options::UNIT_SYSTEM, unitSystem);

    // ... assign pressure conversion factor

    setPressureFactor(unitSystem, options);

    // ... assign conversion factors for other quantities

    setOtherFactors(unitSystem);
}

//-----------------------------------------------------------------------------

//  Set flow units conversion factor

int Units::setFlowFactor(const int flowUnits)
{
    int    unitSystem = Options::US;
    double flowFactor = 1.0;
    string s = "CFS";

    switch(flowUnits)
    {
    case Options::CFS:  flowFactor = 1.0;
                        s = "CFS";
                        break;
    case Options::GPM:  flowFactor = GPMperCFS;
                        s = "GPM";
                        break;
    case Options::MGD:  flowFactor = MGDperCFS;
                        s = "MGD";
                        break;
    case Options::IMGD: flowFactor = IMGDperCFS;
                        s = "IMGD";
                        break;
    case Options::AFD:  flowFactor = AFDperCFS;
                        s = "AFD";
                        break;

    case Options::LPS:  flowFactor = LPSperCFS;
                        s = "LPS";
                        unitSystem = Options::SI;
                        break;
    case Options::LPM:  flowFactor = LPMperCFS;
                        s = "LPM";
                        unitSystem = Options::SI;
                        break;
    case Options::MLD:  flowFactor = MLDperCFS;
                        s = "MLD";
                        unitSystem = Options::SI;
                        break;
    case Options::CMH:  flowFactor = CMHperCFS;
                        s = "CMH";
                        unitSystem = Options::SI;
                        break;
    case Options::CMD:  flowFactor = CMDperCFS;
                        s = "CMD";
                        unitSystem = Options::SI;
                        break;
    }
    factors[FLOW] = flowFactor;
    names[FLOW] = s;
    return unitSystem;
}

//-----------------------------------------------------------------------------

//  Set pressure units and conversion factor

void Units::setPressureFactor(const int unitSystem, Options& options)
{
    if (options.pressureUnits() == Options::KPA)
    {
        factors[PRESSURE] = KPAperPSI * PSIperFT;
        names[PRESSURE] = "kpa";
    }

    else if (unitSystem == Options::US)
    {
        options.setOption(Options::PRESSURE_UNITS, Options::PSI);
        factors[PRESSURE] = PSIperFT;
        names[PRESSURE] = "psi";
    }

    else
    {
        options.setOption(Options::PRESSURE_UNITS, Options::METERS);
        factors[PRESSURE] = MperFT;
        names[PRESSURE] = "m";
    }
}

//-----------------------------------------------------------------------------

//  Set conversion factors and names for other unit quantities

void Units::setOtherFactors(int unitSystem)
{
    if (unitSystem == Options::US)
    {
        factors[DIAMETER] = 12.0;
        factors[LENGTH]   = 1.0;
        factors[VOLUME]   = 1.0;
        factors[POWER]    = 1.0;
        factors[VELOCITY] = 1.0;
        factors[HEADLOSS] = 1000.0;

        names[DIAMETER]   = "in";
        names[LENGTH]     = "ft";
        names[VOLUME]     = "ft3";
        names[POWER]      = "hp";
        names[VELOCITY]   = "ft/s";
        names[HEADLOSS]   = "ft/kft";
    }
    else
    {
        factors[DIAMETER] = MperFT * 1000.0;
        factors[LENGTH]   = MperFT;
        factors[VOLUME]   = M3perFT3;
        factors[POWER]    = KWperHP;
        factors[VELOCITY] = MperFT;
        factors[HEADLOSS] = 1000.0;

        names[DIAMETER]   = "mm";
        names[LENGTH]     = "m";
        names[VOLUME]     = "m3";
        names[POWER]      = "kw";
        names[VELOCITY]   = "m/s";
        names[HEADLOSS]   = "m/km";
    }
}

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

//! \file options.h
//! \brief Describes the Options class.

#ifndef OPTIONS_H_
#define OPTIONS_H_

#include "Output/reportfields.h"
#include <string>
#include <sstream>

class Network;

//! \class Options
//! \brief User-supplied options for analyzing a pipe network.

class Options
{
  public:

    // ... Enumerated values for categorical options

    enum UnitSystem    {US, SI};
    enum FlowUnits     {CFS, GPM, MGD, IMGD, AFD, LPS, LPM, MLD, CMH, CMD};
    enum PressureUnits {PSI, METERS, KPA};
    enum FileMode      {SCRATCH, USE, SAVE};
    enum IfUnbalanced  {STOP, CONTINUE};
    enum QualType      {NOQUAL, AGE, TRACE, CHEM};
    enum QualUnits     {NOUNITS, HRS, PCNT, MGL, UGL};
    enum ReportedItems {NONE, ALL, SOME};

    // ... Options with string values

    enum StringOption {
        HYD_FILE_NAME,         //!< Name of binary file containing hydraulic results
        OUT_FILE_NAME,         //!< Name of binary file containing simulation results
        RPT_FILE_NAME,         //!< Name of text file containing output report
        MAP_FILE_NAME,         //!< Name of text file containing nodal coordinates

        HEADLOSS_MODEL,        //!< Name of head loss model used
        DEMAND_MODEL,          //!< Name of nodal demand model used
        LEAKAGE_MODEL,         //!< Name of pipe leakage model used
        HYD_SOLVER,            //!< Name of hydraulic solver method
        STEP_SIZING,           //!< Name of Newton step size method
        MATRIX_SOLVER,         //!< Name of sparse matrix eqn. solver
        DEMAND_PATTERN_NAME,   //!< Name of global demand pattern

        QUAL_MODEL,            //!< Name of water quality model used
        QUAL_NAME,             //!< Name of water quality constituent
        QUAL_UNITS_NAME,       //!< Name of water quality units
        TRACE_NODE_NAME,       //!< Name of node for source tracing

        MAX_STRING_OPTIONS
    };

    // ... Options with integer, categorical or yes/no values

    enum IndexOption {
        UNIT_SYSTEM,           //!< Choice of units system
        FLOW_UNITS,            //!< Choice of flow rate units
        PRESSURE_UNITS,        //!< Choice of pressure units
        MAX_TRIALS,            //!< Maximum hydraulic trials
        IF_UNBALANCED,         //!< Stop or continue if network is unbalanced
        HYD_FILE_MODE,         //!< Binary hydraulics file mode
        DEMAND_PATTERN,        //!< Global demand pattern index
        ENERGY_PRICE_PATTERN,  //!< Global energy price pattern index

        QUAL_TYPE,             //!< Type of water quality analysis
        QUAL_UNITS,            //!< Units of the quality constituent
        TRACE_NODE,            //!< Node index for source tracing

        REPORT_SUMMARY,        //!< report input/output summary
        REPORT_ENERGY,         //!< report energy usage
        REPORT_STATUS,         //!< report system status
        REPORT_TRIALS,         //!< report result of each trial
        REPORT_NODES,          //!< report node results
        REPORT_LINKS,          //!< report link results

        MAX_INDEX_OPTIONS
    };

    // ... Options with numerical values

    enum ValueOption {

        // Hydraulic properties
        SPEC_GRAVITY,          //!< Specific Gravity
        KIN_VISCOSITY,         //!< Kinematic viscosity (ft2/sec)
        DEMAND_MULTIPLIER,     //!< Global base demand multiplier
        MINIMUM_PRESSURE,      //!< Global minimum pressure to supply demand (ft)
        SERVICE_PRESSURE,      //!< Global pressure to supply full demand (ft)
        PRESSURE_EXPONENT,     //!< Global exponent for power function demands
        EMITTER_EXPONENT,      //!< Global exponent in emitter discharge formula
        LEAKAGE_COEFF1,
        LEAKAGE_COEFF2,

        // Hydraulic tolerances
        RELATIVE_ACCURACY,     //!< sum of all |flow changes| / sum of all |flows|
        HEAD_TOLERANCE,        //!< Convergence tolerance for head loss balance
        FLOW_TOLERANCE,        //!< Convergence tolerance for flow balance
        FLOW_CHANGE_LIMIT,     //!< Max. flow change for convergence
        TIME_WEIGHT,           //!< Time weighting for variable head tanks

        // Water quality options
        MOLEC_DIFFUSIVITY,     //!< Chemical's molecular diffusivity (ft2/sec)
        QUAL_TOLERANCE,        //!< Tolerance for water quality comparisons
        BULK_ORDER,            //!< Order of all bulk flow reactions in pipes
        WALL_ORDER,            //!< Order of all pipe wall reactions
        TANK_ORDER,            //!< Order of all bulk water reactions in tanks
        BULK_COEFF,            //!< Global rate coefficient for bulk reactions
        WALL_COEFF,            //!< Global rate coefficient for wall reactions
        LIMITING_CONCEN,       //!< Maximum concentration for growth reactions
        ROUGHNESS_FACTOR,      //!< Relates wall reaction coeff. to pipe roughness

        // Energy options
        ENERGY_PRICE,          //!< Global energy price (per kwh)
        PEAKING_CHARGE,        //!< Fixed energy charge per peak kw
        PUMP_EFFICIENCY,       //!< Global pump efficiency (fraction)

        MAX_VALUE_OPTIONS
    };

    // ... Time options (in integer seconds)

    enum TimeOption {
        START_TIME,            //!< Starting time of day
        HYD_STEP,              //!< Hydraulic simulation time step
        QUAL_STEP,             //!< Water quality simulation time step
        PATTERN_STEP,          //!< Global time interval for time patterns
        PATTERN_START,         //!< Time of day at which all time patterns start
        REPORT_STEP,           //!< Reporting time step
        REPORT_START,          //!< Simulation time at which reporting begins
        RULE_STEP,             //!< Time step used to evaluate control rules
        TOTAL_DURATION,        //!< Total simulation duration
        REPORT_STATISTIC,      //!< How results are reported (min, max, range)

        MAX_TIME_OPTIONS
    };

    //... Constructor / Destructor

    Options();
    ~Options() {}

    // ... Methods that return an option's value

    int         flowUnits();
    int         pressureUnits();
    std::string stringOption(StringOption option);
    int         indexOption(IndexOption option);
    double      valueOption(ValueOption option);
    int         timeOption(TimeOption option);

    // ... Methods that set an option's value

    void   setDefaults();
    void   adjustOptions();
    int    setOption(StringOption option, const std::string& value);
    int    setOption(IndexOption option, const std::string& value, Network* nw);
    void   setOption(IndexOption option, int value);
    void   setOption(ValueOption option, double value);
    void   setOption(TimeOption option, int value);
    void   setReportFieldOption(int fieldType,
                                int fieldIndex,
                                int enabled,
                                int precision,
                                double lowerLimit,
                                double upperLimit);

    // ... Methods that write a collection of options to a string

    std::string hydOptionsToStr();
    std::string qualOptionsToStr();
    std::string demandOptionsToStr();
    std::string timeOptionsToStr();
    std::string reactOptionsToStr();
    std::string energyOptionsToStr(Network* network);
    std::string reportOptionsToStr();

  private:

    std::string  stringOptions[MAX_STRING_OPTIONS];
    int          indexOptions[MAX_INDEX_OPTIONS];
    double       valueOptions[MAX_VALUE_OPTIONS];
    int          timeOptions[MAX_TIME_OPTIONS];
    ReportFields reportFields;
};

//-----------------------------------------------------------------------------
//    Inline Functions
//-----------------------------------------------------------------------------

inline int Options::flowUnits()
       { return indexOptions[FLOW_UNITS]; }

inline int Options::pressureUnits()
       { return indexOptions[PRESSURE_UNITS]; }

inline std::string Options::stringOption(StringOption option)
       { return stringOptions[option]; }

inline int Options::indexOption(IndexOption option)
       { return indexOptions[option]; }

inline double Options::valueOption(ValueOption option)
       { return valueOptions[option]; }

inline int Options::timeOption(TimeOption option)
       { return timeOptions[option]; }

#endif

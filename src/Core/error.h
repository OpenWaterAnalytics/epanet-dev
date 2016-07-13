/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file error.h
//! \brief Definitions of the EPANET error classes.

#ifndef ERROR_H_
#define ERROR_H_

#include <string>

//-----------------------------------------------------------------------------
//  Abstract exception class
//-----------------------------------------------------------------------------
class ENerror
{
  public:
    ENerror();
    virtual ~ENerror() = 0;

    int code;
    std::string msg;
};

//-----------------------------------------------------------------------------
//  System error exception class
//-----------------------------------------------------------------------------
class SystemError : public ENerror
{
  public:
    enum SystemErrors {
        OUT_OF_MEMORY,                 //101
        NO_NETWORK_DATA,               //102

        HEADLOSS_MODEL_NOT_OPENED,     //103
        DEMAND_MODEL_NOT_OPENED,       //104
        LEAKAGE_MODEL_NOT_OPENED,      //105
        QUALITY_MODEL_NOT_OPENED,      //106
        MATRIX_SOLVER_NOT_OPENED,      //107

        HYDRAULIC_SOLVER_NOT_OPENED,   //108
        QUALITY_SOLVER_NOT_OPENED,     //109
        HYDRAULICS_SOLVER_FAILURE,     //110
        QUALITY_SOLVER_FAILURE,        //111
        SOLVER_NOT_INITIALIZED,        //112
        SYSTEM_ERROR_LIMIT
    };
    SystemError(int type);
};

//-----------------------------------------------------------------------------
//  Input error exception class
//-----------------------------------------------------------------------------
class InputError : public ENerror
{
  public:
    enum InputErrors {
        ERRORS_IN_INPUT_DATA,          //200
        CANNOT_CREATE_OBJECT,          //201
        TOO_FEW_ITEMS,                 //202
        INVALID_KEYWORD,               //203
        DUPLICATE_ID,                  //204
        UNDEFINED_OBJECT,              //205
        INVALID_NUMBER,                //206
        INVALID_TIME,                  //207
        UNSPECIFIED,                   //208
        INPUT_ERROR_LIMIT
    };
    InputError(int type, std::string token);
};

//-----------------------------------------------------------------------------
//  Network error class
//-----------------------------------------------------------------------------
class NetworkError : public ENerror
{
  public:
    enum NetworkErrors {
        ILLEGAL_VALVE_CONNECTION,      //220
        TOO_FEW_NODES,                 //223
        NO_FIXED_GRADE_NODES,          //224
        INVALID_TANK_LEVELS,           //225
        NO_PUMP_CURVE,                 //226
        INVALID_PUMP_CURVE,            //227
        INVALID_CURVE_DATA,            //230
        INVALID_VOLUME_CURVE,          //231
        UNCONNECTED_NODE,              //233
        NETWORK_ERROR_LIMIT
    };
    NetworkError(int type, std::string id);
};

//-----------------------------------------------------------------------------
//  File error exception class
//-----------------------------------------------------------------------------
class FileError : public ENerror
{
  public:
    enum FileErrors {
        DUPLICATE_FILE_NAMES,          //301
        CANNOT_OPEN_INPUT_FILE,        //302
        CANNOT_OPEN_REPORT_FILE,       //303
        CANNOT_OPEN_OUTPUT_FILE,       //304
        CANNOT_OPEN_HYDRAULICS_FILE,   //305
        INCOMPATIBLE_HYDRAULICS_FILE,  //306
        CANNOT_READ_HYDRAULICS_FILE,   //307
        CANNOT_WRITE_TO_OUTPUT_FILE,   //308
        CANNOT_WRITE_TO_REPORT_FILE,   //309
        NO_RESULTS_SAVED_TO_REPORT,    //310
        FILE_ERROR_LIMIT
    };
    FileError(int type);
};

#endif

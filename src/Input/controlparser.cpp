/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "controlparser.h"
#include "Core/network.h"
#include "Core/error.h"
#include "Elements/tank.h"
#include "Elements/link.h"
#include "Elements/control.h"
#include "Utilities/utilities.h"

using namespace std;

//-----------------------------------------------------------------------------
//  Control Keywords
//-----------------------------------------------------------------------------
static const char* w_OPEN      = "OPEN";
static const char* w_CLOSED    = "CLOSED";
static const char* w_NODE      = "NODE";
static const char* w_TIME      = "TIME";
static const char* w_CLOCKTIME = "CLOCKTIME";
static const char* w_ABOVE     = "ABOVE";
static const char* w_BELOW     = "BELOW";
static const char* w_LINK      = "LINK-";

//-----------------------------------------------------------------------------

ControlParser::ControlParser()
{
    initializeControlSettings();
}

//-----------------------------------------------------------------------------

void ControlParser::parseControlLine(string& line, Network* network)

// Formats are:
//   LINK id OPEN/CLOSED/setting IF NODE id ABOVE/BELOW value
//   . . .                       AT TIME time
//   . . .                       AT CLOCKTIME time  (AM/PM)
// where time is in decimal hours or hrs:min:sec.

{
    // ... initialize
    initializeControlSettings();

    // ... skip over LINK keyword
    sin.clear();
    sin.str(line);
    string keyword;
    sin >> keyword;

    // ... get settings for link being controlled
    parseLinkSetting(network);

    //... skip next keyword
    sin >> keyword;

    //... get next keyword
    sin >> keyword;
    if (sin.eof()) throw InputError(InputError::TOO_FEW_ITEMS, "");

    //... get node control level if keyword == NODE
    if ( Utilities::match(keyword, w_NODE) ) parseLevelSetting(network);

    //... get control time if keyword == TIME
    else if ( Utilities::match(keyword, w_TIME) ) parseTimeSetting();

    //... get control time of day if keyword == CLOCKTIME
    else if ( Utilities::match(keyword, w_CLOCKTIME) ) parseTimeOfDaySetting();

    else throw InputError(InputError::INVALID_KEYWORD, keyword);

    //... create the control
    createControl(network);
}

//-----------------------------------------------------------------------------

void ControlParser::initializeControlSettings()
{
    //... initialize control parameters
    controlType = -1;
    link = 0;
    linkStatus = Control::NO_STATUS;
    linkSetting = 0.0;
    node = 0;
    levelType = Control::LOW_LEVEL;
    nodeSetting = 0.0;
    timeSetting = -1;
}

//-----------------------------------------------------------------------------

void ControlParser::parseLinkSetting(Network* network)
{
    //... read id of link being controlled
    string id;
    sin >> id;
    link = network->link(id);
    if (link == nullptr) throw InputError(InputError::UNDEFINED_OBJECT, id);

    //... read control setting/status as a string
    if ( sin.eof() ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string settingStr;
    sin >> settingStr;

    //... convert string to status or to numerical value
    if ( Utilities::match(settingStr, w_OPEN) )
    {
        linkStatus = Control::OPEN_STATUS;
    }
    else if ( Utilities::match(settingStr, w_CLOSED) )
    {
        linkStatus = Control::CLOSED_STATUS;
    }
    else if ( !Utilities::parseNumber(settingStr, linkSetting) )
    {
        throw InputError(InputError::INVALID_NUMBER, settingStr);
    }
}

//-----------------------------------------------------------------------------

void ControlParser::parseLevelSetting(Network* network)
{
    // ... read id of node triggering the control
    string id;
    sin >> id;
    node = network->node(id);
    if (node == nullptr) throw InputError(InputError::UNDEFINED_OBJECT, id);

    // ... get type of trigger level
    if ( sin.eof() ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string keyword;
    sin >> keyword;
    if ( Utilities::match(keyword, w_ABOVE) ) levelType = Control::HI_LEVEL;
    else if (Utilities::match(keyword, w_BELOW)) levelType = Control::LOW_LEVEL;
    else throw InputError(InputError::INVALID_KEYWORD, keyword);

    // ... get trigger level
    if ( sin.eof() ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    string settingStr;
    sin >> settingStr;
    if ( !Utilities::parseNumber(settingStr, nodeSetting) )
    {
        throw InputError(InputError::INVALID_NUMBER, settingStr);
    }

    // ... get control type
    if ( node->type() == Node::TANK ) controlType = Control::TANK_LEVEL;
    else controlType = Control::PRESSURE_LEVEL;
}

//-----------------------------------------------------------------------------

void ControlParser::parseTimeSetting()
{
    // ... read elapsed time and optional time units
    string strTime;
    string strUnits = "";
    sin >> strTime;
    if ( !sin.eof() ) sin >> strUnits;

    // ... convert time string to seconds
    timeSetting = Utilities::getSeconds(strTime, strUnits);
    if ( timeSetting < 0 )
    {
        throw InputError(InputError::INVALID_TIME, strTime + " " + strUnits);
    }
    controlType = Control::ELAPSED_TIME;
}

//-----------------------------------------------------------------------------

void ControlParser::parseTimeOfDaySetting()
{
    // ... read time of day
    string strTime;
    string strUnits = "";
    if ( sin.eof() ) throw InputError(InputError::TOO_FEW_ITEMS, "");
    sin >> strTime;
    if ( !sin.eof() ) sin >> strUnits;

    // ... convert time of day to seconds
    timeSetting = Utilities::getSeconds(strTime, strUnits);
    if ( timeSetting < 0 )
    {
        throw InputError(InputError::INVALID_TIME, strTime + " " + strUnits);
    }
    controlType = Control::TIME_OF_DAY;
}

//-----------------------------------------------------------------------------

void ControlParser::createControl(Network* network)
{
    //... add a new control to the network
    string name = w_LINK + link->name;
    if ( !network->addElement(Element::CONTROL, controlType, name) )
    {
        throw InputError(InputError::CANNOT_CREATE_OBJECT, name + " control");
    }
    int last = network->count(Element::CONTROL) - 1;
    Control* control = network->control(last);

    //... set the control's parameters
    control->setProperties(
        controlType, link, linkStatus, linkSetting, node, nodeSetting, levelType, timeSetting);
}

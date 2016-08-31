/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "control.h"
#include "link.h"
#include "tank.h"
#include "Core/network.h"
#include "Core/error.h"
#include "Utilities/utilities.h"

#include <cmath>
#include <sstream>

using namespace std;

//-----------------------------------------------------------------------------

static const string s_TankLevel      = " by level control on tank ";
static const string s_PressureLevel  = " by pressure control on node ";
static const string s_ElapsedTime    = " by elapsed time control";
static const string s_TimeOfDay      = " by time of day control";
static const string s_StatusChanged  = " status changed to ";
static const string s_SettingChanged = " setting changed to ";
static const string statusTxt[]      = {"closed", "open", ""};

//-----------------------------------------------------------------------------

Control::Control(int type_, string name_) :
    Element(name_),
    type(type_),
    link(nullptr),
    status(NO_STATUS),
    setting(0.0),
    node(nullptr),
    head(0.0),
    volume(0.0),
    levelType(NO_LEVEL),
    time(0)
{
}

Control::~Control() {}

//-----------------------------------------------------------------------------

void Control::setProperties(int    controlType,
                            Link*  controlLink,
                            int    linkStatus,
                            double linkSetting,
                            Node*  controlNode,
                            double nodeSetting,
                            int    controlLevelType,
                            int    timeSetting)
{
    type = controlType;
    link = controlLink;
    if ( linkStatus != NO_STATUS ) status = linkStatus;
    else setting = linkSetting;
    node = controlNode;
    head = nodeSetting;
    levelType = (LevelType)controlLevelType;
    time = timeSetting;
}

//-----------------------------------------------------------------------------

void Control::convertUnits(Network* network)
{
    if ( type == TANK_LEVEL )
    {
        Tank* tank = static_cast<Tank*>(node);
        head = head / network->ucf(Units::LENGTH) + node->elev;
        volume = tank->findVolume(head);
    }

    else if ( type == PRESSURE_LEVEL )
    {
        head = head / network->ucf(Units::PRESSURE) + node->elev;
    }

    if ( link ) setting = link->convertSetting(network, setting);
}

//-----------------------------------------------------------------------------

int Control::timeToActivate(Network* network, int t, int tod)
{
    Tank* tank;
    bool makeChange = false; //do not implement any control actions
    int  aTime = -1;
    switch (type)
    {
    case PRESSURE_LEVEL: break;

    case TANK_LEVEL:
        tank = static_cast<Tank*>(node);
        aTime = tank->timeToVolume(volume);
        break;

    case ELAPSED_TIME:
        aTime = time - t;
        break;

    case TIME_OF_DAY:
        if (time >= tod) aTime = time - tod;
        else aTime = 86400 - tod + time;
        break;
    }

    if ( aTime > 0 && activate(makeChange, network->msgLog) ) return aTime;
    else return -1;
}

//-----------------------------------------------------------------------------

void Control::applyPressureControls(Network* network)
{
    bool makeChange = true;

    for (Control* control : network->controls)
    {
        if ( control->type == PRESSURE_LEVEL )
        {
            if ( (control->levelType == LOW_LEVEL &&
                  control->node->head < control->head)
            ||   (control->levelType == HI_LEVEL &&
                  control->node->head > control->head) )
            {
                control->activate(makeChange, network->msgLog);
            }
        }
    }
}

//-----------------------------------------------------------------------------

void Control::apply(Network* network, int t, int tod)
{
    bool makeChange = true;
    Tank* tank;

    switch (type)
    {
    case PRESSURE_LEVEL: break;

    case TANK_LEVEL:
        tank = static_cast<Tank*>(node);
        // ... use tolerance of one second's worth of inflow/outflow on action level
        if ( (levelType == LOW_LEVEL && tank->volume <= volume + abs(tank->outflow) )
        ||   (levelType == HI_LEVEL && tank->volume >= volume - abs(tank->outflow)) )
        {
            activate(makeChange, network->msgLog);
        }
        break;

    case ELAPSED_TIME:
        if ( t == time ) activate(makeChange, network->msgLog);
        break;

    case TIME_OF_DAY:
        if ( tod == time ) activate(makeChange, network->msgLog);
        break;
    }
}

//-----------------------------------------------------------------------------

bool Control::activate(bool makeChange, ostream& msgLog)
{
    bool   result = false;
    string reason = "";
    string linkStr = link->typeStr() + " " + link->name;

    switch (type)
    {
    case TANK_LEVEL:
        reason = s_TankLevel + node->name;
        break;

    case PRESSURE_LEVEL:
        reason = s_PressureLevel + node->name;
        break;

    case ELAPSED_TIME:
        reason = s_ElapsedTime;
        break;

    case TIME_OF_DAY:
        reason = s_TimeOfDay;
        break;
    }

    if ( status != NO_STATUS )
    {
        reason =  linkStr + s_StatusChanged + statusTxt[status] + reason;
        result = link->changeStatus(status, makeChange, reason, msgLog);
    }
    else
    {
        reason = linkStr + s_SettingChanged + Utilities::to_string(setting) +
            reason;
        result = link->changeSetting(setting, makeChange, reason, msgLog);
    }
    return result;
}

//-----------------------------------------------------------------------------

string Control::toStr(Network* nw)
{
    // ... write Link name and its control action

    stringstream s;
    s << "Link " << link->name << " ";
    if ( status == CLOSED_STATUS )    s << "CLOSED";
    else if ( status == OPEN_STATUS ) s << "OPEN";
    else s << " " << setting << " ";

    // ... write node condition or time causing the action

    switch (type)
    {
    case TANK_LEVEL:
        s << " IF NODE " << node->name << " ";
        if ( levelType == LOW_LEVEL ) s << "BELOW";
        else s << "ABOVE";
        s << " " << (head - node->elev) * nw->ucf(Units::LENGTH);
        break;

    case PRESSURE_LEVEL:
        s << " IF NODE " << node->name << " ";
        if ( levelType == LOW_LEVEL ) s << "BELOW";
        else s << "ABOVE";
        s << " " << (head - node->elev) * nw->ucf(Units::PRESSURE);
        break;

    case ELAPSED_TIME:
        s << " AT TIME " <<  Utilities::getTime(time);
        break;

    case TIME_OF_DAY:
        s << " AT CLOCKTIME " << Utilities::getTime(time);
        break;
    }
    return s.str();
}

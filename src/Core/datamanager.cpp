/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

 ////////////////////////////////////////////////
 //  Implementation of the DataManager class.  //
 ////////////////////////////////////////////////

 // TO DO:
 // - finish implementing the functions declared in EPANET3.H.
 // - provide a brief comment on what each function does

#include "datamanager.h"
#include "Core/network.h"
#include "Core/error.h"
#include "Core/constants.h"
#include "Elements/node.h"
#include "Elements/junction.h"
#include "Elements/tank.h"
#include "Elements/link.h"
#include "Elements/pipe.h"
#include "Elements/valve.h"
#include "Elements/curve.h"
#include "Elements/pattern.h"
#include "Elements/qualsource.h"
#include "epanet3.h"

#include "cstring"
using namespace std;

int getTankValue(int param, Node* node, double* value, Network* nw);
int getQualSourceValue(int param, Node* node, double *value, Network* nw);
int getPipeValue(int param, Link* link, double* value, Network* nw);

//-----------------------------------------------------------------------------

int DataManager::getCount(int element, int* count, Network* nw)
{
    int err = 0;
    *count = 0;
    switch (element)
    {
    case EN_NODECOUNT:    *count = nw->count(Element::NODE); break;
    case EN_LINKCOUNT:    *count = nw->count(Element::LINK); break;
    case EN_PATCOUNT:     *count = nw->count(Element::PATTERN); break;
    case EN_CURVECOUNT:   *count = nw->count(Element::CURVE); break;
    case EN_CONTROLCOUNT: *count = nw->count(Element::CONTROL); break;
    case EN_RULECOUNT:    break;
    case EN_TANKCOUNT:
        for (Node* node : nw->nodes) if ( node->type() == Node::TANK ) (*count)++;
        break;
    case EN_RESVCOUNT:
        for (Node* node : nw->nodes) if ( node->type() == Node::RESERVOIR ) (*count)++;
         break;
    default: err = 203;
    }
    return err;
}

//-----------------------------------------------------------------------------

int DataManager::getNodeIndex(char* name, int* index, Network* nw)
{
    *index = nw->indexOf(Element::NODE, name);
    if ( *index < 0 ) return 205;
    return 0;
}

//-----------------------------------------------------------------------------

int DataManager::getNodeId(int index, char* id, Network* nw)
{
    if ( index < 0 || index >= nw->count(Element::NODE) )
    {
        strcpy(id, "");
        return 205;
    }
    strcpy(id, nw->node(index)->name.c_str());
    return 0;
}

//-----------------------------------------------------------------------------

int DataManager::getNodeType(int index, int* type, Network* nw)
{
    *type = 0;
    if ( index < 0 || index >= nw->count(Element::NODE) ) return 205;
    switch (nw->node(index)->type())
    {
        case Node::JUNCTION:  *type = EN_JUNCTION;  break;
        case Node::RESERVOIR: *type = EN_RESERVOIR; break;
        case Node::TANK:      *type = EN_TANK;      break;
    }
    return 0;
}

//-----------------------------------------------------------------------------

int DataManager::getNodeValue(int index, int param, double* value, Network* nw)
{
    *value = 0.0;
    if ( index < 0 || index >= nw->count(Element::NODE) ) return 205;

    double lcf = nw->ucf(Units::LENGTH);
    double pcf = nw->ucf(Units::PRESSURE);
    double qcf = nw->ucf(Units::FLOW);
    double ccf = nw->ucf(Units::CONCEN);

    Node* node = nw->node(index);
    double dummy = 0.0;
    switch (param)
    {
    case EN_ELEVATION:
        *value = node->elev * lcf;
        break;

    case EN_BASEDEMAND:   break;
    case EN_BASEPATTERN:      break;

    case EN_FULLDEMAND:
        *value = node->fullDemand * qcf;
        break;

    case EN_ACTUALDEMAND:
        *value = node->actualDemand * qcf;
        break;

    case EN_OUTFLOW:
        *value = node->outflow * qcf;
         if ( node->type() != Node::JUNCTION ) *value = -(*value);
         break;

    case EN_EMITTERFLOW:
        *value = node->findEmitterFlow(node->head, dummy) * qcf;
        break;

    case EN_HEAD:
        *value = node->head * lcf;
        break;

    case EN_PRESSURE:
        *value = (node->head - node->elev) * pcf;
        break;

    case EN_INITQUAL:
        *value = node->initQual * ccf;
        break;

    case EN_QUALITY:
        *value = node->quality * ccf;
        break;

    case EN_SOURCEQUAL:
    case EN_SOURCEPAT:
    case EN_SOURCETYPE:
    case EN_SOURCEMASS:
        return getQualSourceValue(param, node, value, nw);

    // ... remaining node parameters apply only to Tanks
    default: return getTankValue(param, node, value, nw);
    }
    return 0;
}

//-----------------------------------------------------------------------------

int DataManager::getLinkIndex(char* name, int* index, Network* nw)
{
    *index = nw->indexOf(Element::LINK, name);
    if ( *index < 0 ) return 205;
    return 0;
}

//-----------------------------------------------------------------------------

int DataManager::getLinkId(int index, char* id, Network* nw)
{
    if ( index < 0 || index >= nw->count(Element::LINK) )
    {
        strcpy(id, "");
        return 205;
    }
    strcpy(id, nw->link(index)->name.c_str());
    return 0;
}

//-----------------------------------------------------------------------------

int DataManager::getLinkType(int index, int* type, Network* nw)
{
    *type = EN_PIPE;
    if ( index < 0 || index >= nw->count(Element::LINK) ) return 205;
    Link* link = nw->link(index);
    if ( link->type() == Link::PIPE )
    {
        Pipe* pipe = static_cast<Pipe*>(link);
        if ( pipe->hasCheckValve ) *type = EN_CVPIPE;
        else                       *type = EN_PIPE;
    }
    else if (link->type() == Link::PUMP ) *type = EN_PUMP;
    else if ( link->type() == Link::VALVE )
    {
        Valve* valve = static_cast<Valve*>(link);
        *type = valve->type() + EN_PUMP;
    }
    return 0;
}

//-----------------------------------------------------------------------------

int DataManager::getLinkNodes(int index, int* fromNode, int* toNode, Network* nw)
{
    *fromNode = -1;
    *toNode = -1;
    if ( index < 0 || index >= nw->count(Element::LINK) ) return 205;
    *fromNode = nw->link(index)->fromNode->index;
    *toNode = nw->link(index)->toNode->index;
    return 0;
}

//-----------------------------------------------------------------------------

int DataManager::getLinkValue(int index, int param, double* value, Network* nw)
{
    *value = 0.0;
    if ( index < 0 || index >= nw->count(Element::LINK) ) return 205;
    Link* link = nw->link(index);
    switch (param)
    {
    case EN_DIAMETER:
        *value = link->diameter * nw->ucf(Units::DIAMETER);
        break;
     case EN_MINORLOSS:
         *value = link->lossCoeff;
         break;
     case EN_INITSTATUS:
        *value = link->initStatus;
        break;
    case EN_INITSETTING:
        *value = link->initSetting;
        break;
    case EN_FLOW:
        *value = link->flow * nw->ucf(Units::FLOW);
        break;
    case EN_VELOCITY:
        *value = link->getVelocity() * nw->ucf(Units::LENGTH);
        break;
    case EN_HEADLOSS:
        *value = link->hLoss * nw->ucf(Units::LENGTH);
        break;
    case EN_STATUS:
        *value = link->status;
        break;
    case EN_SETTING:
        *value = link->getSetting(nw);
        break;
    case EN_ENERGY:
        break;                         // TO BE ADDED
    case EN_LINKQUAL:
        *value = link->quality * nw->ucf(Units::CONCEN);
        break;
    case EN_LEAKAGE:
        *value = link->leakage * nw->ucf(Units::FLOW);
        break;
    default: return getPipeValue(param, link, value, nw);
    }
    return 0;
}

//-----------------------------------------------------------------------------

int getTankValue(int param, Node* node, double* value, Network* nw)
{
    double lcf = nw->ucf(Units::LENGTH);
    double vcf = lcf * lcf * lcf;
    *value = 0.0;
    if ( node->type() != Node::TANK ) return 0;
    Tank* tank = static_cast<Tank*>(node);
    switch (param)
    {
    case EN_TANKLEVEL:
        *value = (tank->head - tank->elev) * lcf;
        break;
    case EN_INITVOLUME:
        *value = tank->findVolume(tank->head) * vcf;
        break;
    case EN_MIXMODEL:
        *value = tank->mixingModel.type;
        break;
    case EN_MIXZONEVOL:
        *value = (tank->mixingModel.fracMixed * tank->maxVolume) * vcf;
        break;
    case EN_TANKDIAM:
        *value = tank->diameter * lcf;
        break;
    case EN_MINVOLUME:
        *value = tank->minVolume * vcf;
        break;
    case EN_VOLCURVE:
        if ( tank->volCurve )
        {
            string name = tank->volCurve->name;
            int index = nw->indexOf(Element::CURVE, name);
            *value = index;
            if ( index < 0 ) return 205;
        }
        else *value = -1.0;
        break;
    case EN_MINLEVEL:
        *value = (tank->minHead - tank->elev) * lcf;
        break;
    case EN_MAXLEVEL:
        *value = (tank->maxHead - tank->elev) * lcf;
        break;
    case EN_MIXFRACTION:
        *value = tank->mixingModel.fracMixed;
        break;
    case EN_TANK_KBULK:
        *value = tank->bulkCoeff ;
        break;
    case EN_TANKVOLUME:
        *value = tank->volume * vcf;
        break;
    default: return 203;
    }
    return 0;
}

//-----------------------------------------------------------------------------

int getQualSourceValue(int param, Node* node, double *value, Network* nw)
{
    *value = 0.0;
    if ( node->qualSource )
    {
        switch (param)
        {
            // ... base quality is stored in user units
            case EN_SOURCEQUAL: *value = node->qualSource->base; break;
            case EN_SOURCEPAT:
                if ( node->qualSource->pattern)
                {
                    string name = node->qualSource->pattern->name;
                    int index = nw->indexOf(Element::PATTERN, name);
                    *value = index;
                    if ( index < 0 ) return 205;
                }
                break;
            case EN_SOURCETYPE: *value = node->qualSource->type; break;
            case EN_SOURCEMASS:
                if ( node->qualSource->type == QualSource::MASS )
                    *value = node->qualSource->strength / 60.0;
                else *value = node->qualSource->strength * FT3perL;
                break;
        }
    }
    else if ( param == EN_SOURCEPAT ) *value = -1;
    return 0;
}

//-----------------------------------------------------------------------------

int getPipeValue(int param, Link* link, double* value, Network* nw)
{
    double lcf = nw->ucf(Units::LENGTH);
    *value = 0.0;
    if ( link->type() != Link::PIPE ) return 0;
    Pipe* pipe = static_cast<Pipe*>(link);
    switch (param)
    {
    case EN_LENGTH:
        *value = pipe->length * lcf;
        break;
    case EN_ROUGHNESS:
        *value = pipe->roughness;
        if ( nw->option(Options::HEADLOSS_MODEL) == "D-W") *value *= 1000.0 * lcf;
        break;
    case EN_KBULK:
        *value = pipe->bulkCoeff;
        break;
    case EN_KWALL:
        *value = pipe->wallCoeff;
        break;
    case EN_LEAKCOEFF1:
        *value = pipe->leakCoeff1;
        break;
    case EN_LEAKCOEFF2:
        *value = pipe->leakCoeff2;
    default: return 203;
    }
    return 0;
}

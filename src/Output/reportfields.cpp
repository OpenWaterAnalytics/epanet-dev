/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "reportfields.h"
#include "Elements/element.h"

#include <limits>
using namespace std;

const char* ReportFields::NodeFieldNames[] =
    {"Elevation", "Head", "Pressure", "Demand", "Deficit", "Outflow", "Quality", 0};

const char* ReportFields::LinkFieldNames[] =
    {"Length", "Diameter", "Flow Rate", "Leakage", "Velocity", "Head Loss",
     "Status", "Setting", "Quality", 0};

ReportFields::ReportFields()
{
    setDefaults();
}

void ReportFields::setDefaults()
{
    for (int i = 0; i < NUM_NODE_FIELDS; i++)
    {
        nodeFields[i].name = NodeFieldNames[i];
        nodeFields[i].precision = 3;
        nodeFields[i].lowerLimit = numeric_limits<double>::min();
        nodeFields[i].upperLimit = numeric_limits<double>::max();
    }
    for (int i = 0; i < NUM_LINK_FIELDS; i++)
    {
        linkFields[i].name = LinkFieldNames[i];
        linkFields[i].precision = 3;
        linkFields[i].lowerLimit = numeric_limits<double>::max();
        linkFields[i].upperLimit = numeric_limits<double>::min();
    }
}

void ReportFields::setField(int    type,
                            int    index,
                            int    enabled,
                            int    precision,
                            double lowerLimit,
                            double upperLimit)
{
    Field* field;
    if ( type == Element::NODE )
    {
        if ( index < 0 || index >= NUM_NODE_FIELDS ) return;
        field = &nodeFields[index];
    }
    else if ( type == Element::LINK )
    {
        if ( index < 0 || index >= NUM_LINK_FIELDS ) return;
        field = &linkFields[index];
    }
    else return;
    if ( enabled >= 0 ) field->enabled = enabled;
    if ( precision >= 0 ) field->precision = precision;
    if ( lowerLimit >= 0.0 ) field->lowerLimit = lowerLimit;
    if ( upperLimit >= 0.0 ) field->upperLimit = upperLimit;
}


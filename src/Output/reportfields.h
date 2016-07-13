/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#ifndef REPORTFIELDS_H_
#define REPORTFIELDS_H_

#include <string>

struct Field
{
    std::string name;
    std::string units;
    bool        enabled;
    int         precision;
    double      lowerLimit;
    double      upperLimit;
};


class ReportFields
{
  public:
    enum NodeFieldType {ELEVATION, HEAD, PRESSURE, DEMAND, DEFICIT, OUTFLOW,
                        NODE_QUALITY, NUM_NODE_FIELDS};
    enum LinkFieldType {LENGTH, DIAMETER, FLOW, LEAKAGE, VELOCITY, HEADLOSS, STATUS,
                        SETTING, LINK_QUALITY, NUM_LINK_FIELDS};

    static const char* NodeFieldNames[];
    static const char* LinkFieldNames[];

    ReportFields();
    void setDefaults();
    void setField( int    type,
                   int    index,
                   int    enabled,
                   int    precision,
                   double lowerLimit,
                   double upperLimit);
    Field& nodeField(int index) { return nodeFields[index]; }
    Field& linkField(int index) { return linkFields[index]; }

  private:
    Field nodeFields[NUM_NODE_FIELDS];
    Field linkFields[NUM_LINK_FIELDS];
};

#endif /* REPORTFIELDS_H_ */

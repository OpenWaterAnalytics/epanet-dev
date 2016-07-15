/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file network.h
//! \brief Describes the Network class.

#ifndef NETWORK_H_
#define NETWORK_H_

#include "Core/options.h"
#include "Core/units.h"
#include "Core/qualbalance.h"
#include "Elements/element.h"
#include "Utilities/graph.h"

#include <vector>
#include <ostream>
#include <map>

class Node;
class Link;
class Pattern;
class Curve;
class Control;
class HeadLossModel;
class DemandModel;
class LeakageModel;
class QualModel;
class MemPool;

//! \class Network
//! \brief Contains the data elements that describe a pipe network.
//!
//! A Network object contains collections of the individual elements
//! belonging to the pipe network being analyzed by a Project.

class Network
{
  public:

    Network();
    ~Network();

    // Clears all elements from the network
    void          clear();

    // Adds an element to the network
    bool          addElement(Element::ElementType eType, int subType, std::string name);

    // Finds element counts by type and index by id name
    int           count(Element::ElementType eType);
    int           indexOf(Element::ElementType eType, const std::string& name);

    // Gets an analysis option by type
    int           option(Options::IndexOption type);
    double        option(Options::ValueOption type);
    long          option(Options::TimeOption type);
    std::string   option(Options::StringOption type);

    // Gets a network element by id name
    Node*         node(const std::string& name);
    Link*         link(const std::string& name);
    Pattern*      pattern(const std::string& name);
    Curve*        curve(const std::string& name);
    Control*      control(const std::string& name);

    // Gets a network element by index
    Node*         node(const int index);
    Link*         link(const int index);
    Pattern*      pattern(const int index);
    Curve*        curve(const int index);
    Control*      control(const int index);

    // Creates analysis models
    bool          createHeadLossModel();
    bool          createDemandModel();
    bool          createLeakageModel();
    bool          createQualModel();

    // Network graph theory operations
    Graph         graph;

    // Unit conversions
    double        ucf(Units::Quantity quantity);       //unit conversion factor
    std::string   getUnits(Units::Quantity quantity);  //unit names
    void          convertUnits();

    // Adds/writes network title
    void          addTitleLine(std::string line);
    void          writeTitle(std::ostream& out);

    // Elements of a network
    std::vector<std::string> title;         //!< descriptive title for the network
    std::vector<Node*>       nodes;         //!< collection of node objects
    std::vector<Link*>       links;         //!< collection of link objects
    std::vector<Curve*>      curves;        //!< collection of data curve objects
    std::vector<Pattern*>    patterns;      //!< collection of time pattern objects
    std::vector<Control*>    controls;      //!< collection of control rules
    Units                    units;         //!< unit conversion factors
    Options                  options;       //!< analysis options
    QualBalance              qualBalance;   //!< water quality mass balance
    std::ostringstream       msgLog;        //!< status message log.

    // Computational sub-models
    HeadLossModel*           headLossModel; //!< pipe head loss model
    DemandModel*             demandModel;   //!< nodal demand model
    LeakageModel*            leakageModel;  //!< pipe leakage model
    QualModel*               qualModel;     //!< water quality model

  private:

    // Hash tables that associate an element's ID name with its storage index.
    std::map<std::string, Element*>      nodeTable;     //!< hash table for node ID names.
    std::map<std::string, Element*>      linkTable;     //!< hash table for link ID names.
    std::map<std::string, Element*>      curveTable;    //!< hash table for curve ID names.
    std::map<std::string, Element*>      patternTable;  //!< hash table for pattern ID names.
    std::map<std::string, Element*>      controlTable;  //!< hash table for control ID names.
    MemPool *      memPool;       //!< memory pool for network objects
};

//-----------------------------------------------------------------------------
//    Inline Functions
//-----------------------------------------------------------------------------

// Gets the value of a project option

inline int Network::option(Options::IndexOption type)
       { return options.indexOption(type); }

inline double Network::option(Options::ValueOption type)
       { return options.valueOption(type); }

inline long Network::option(Options::TimeOption type)
       { return options.timeOption(type); }

inline std::string Network::option(Options::StringOption type)
       { return options.stringOption(type); }

// Gets the unit conversion factor (user units per internal units) for a quantity

inline double Network::ucf(Units::Quantity quantity)
       { return units.factor(quantity); }

// Gets the name of the units for a quantity

inline std::string Network::getUnits(Units::Quantity quantity)
       { return units.name(quantity); }

inline void Network::addTitleLine(std::string line)
       { title.push_back(line); }

#endif

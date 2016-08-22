/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

 ////////////////////////////////////////////
 //  Implementation of the Network class.  //
 ////////////////////////////////////////////

#include "network.h"
#include "error.h"
#include "Elements/node.h"
#include "Elements/link.h"
#include "Elements/pattern.h"
#include "Elements/curve.h"
#include "Elements/control.h"
#include "Models/headlossmodel.h"
#include "Models/demandmodel.h"
#include "Models/leakagemodel.h"
#include "Models/qualmodel.h"
#include "Utilities/mempool.h"

using namespace std;

//-----------------------------------------------------------------------------

// Constructor

Network::Network() :
    headLossModel(nullptr),
    demandModel(nullptr),
    leakageModel(nullptr),
    qualModel(nullptr)
{
    options.setDefaults();
    memPool = new MemPool();
}

//-----------------------------------------------------------------------------

// Destructor

Network::~Network()
{
    clear();
    delete memPool;
    memPool = nullptr;
    delete headLossModel;
    headLossModel = nullptr;
    delete demandModel;
    demandModel = nullptr;
    delete leakageModel;
    leakageModel = nullptr;
    delete qualModel;
    qualModel = nullptr;

//    cout << "\nNetwork destructed.\n";
}

//-----------------------------------------------------------------------------

void Network::clear()
{
    // ... destroy all network elements

    for (Node* node : nodes) node->~Node();
    nodes.clear();
    for (Link* link : links) link->~Link();
    links.clear();
    for (Pattern* pattern : patterns) pattern->~Pattern();
    patterns.clear();
    for (Curve* curve : curves) curve->~Curve();
    curves.clear();
    for (Control* control : controls) control->~Control();
    controls.clear();

    // ... reclaim all memory allocated by the memory pool

    memPool->reset();

    // ... re-set all options to their default values

    options.setDefaults();

    // ... delete the contents of the message log
    //msgLog.str("");
}

//-----------------------------------------------------------------------------

int Network::count(Element::ElementType eType)
{
    switch(eType)
    {
    case Element::NODE:    return nodes.size();
    case Element::LINK:    return links.size();
    case Element::PATTERN: return patterns.size();
    case Element::CURVE:   return curves.size();
    case Element::CONTROL: return controls.size();
    }
    return 0;
}

//-----------------------------------------------------------------------------

int Network::indexOf(Element::ElementType eType, const string& name)
{
    map<string,Element*> *table;
    switch(eType)
    {
    case Element::NODE:
        table = &nodeTable;
        break;
    case Element::LINK:
        table = &linkTable;
        break;
    case Element::PATTERN:
        table = &patternTable;
        break;
    case Element::CURVE:
        table = &curveTable;
        break;
    case Element::CONTROL:
        table = &controlTable;
        break;
    default:
        return -1;
    }
    auto it = table->find(name);
    if (it != table->end()) {
      return it->second->index;
    }
  
    return -1;
}

//-----------------------------------------------------------------------------

Node* Network::node(const string& name)
{
    return static_cast<Node*>(nodeTable.find(name)->second);
}

Node* Network::node(const int index)
{
    return nodes[index];
}

//-----------------------------------------------------------------------------

Link* Network::link(const string& name)
{
    return static_cast<Link*>(linkTable.find(name)->second);
}

Link* Network::link(const int index)
{
    return links[index];
}

//-----------------------------------------------------------------------------

Pattern* Network::pattern(const string& name)
{
    return static_cast<Pattern*>(patternTable.find(name)->second);
}

Pattern* Network::pattern(const int index)
{
    return patterns[index];
}

//-----------------------------------------------------------------------------

Curve* Network::curve(const string& name)
{
    return static_cast<Curve*>(curveTable.find(name)->second);
}

Curve* Network::curve(const int index)
{
    return curves[index];
}

//-----------------------------------------------------------------------------

Control*  Network::control(const string& name)
{
    return static_cast<Control*>(controlTable.find(name)->second);
}

Control* Network::control(const int index)
{
    return controls[index];
}

//-----------------------------------------------------------------------------

void Network::writeTitle(ostream& out)
{
    if ( title.size() > 0 )
    {
        out << endl;
        for (string s : title) out << "  " << s << "\n";
    }
}

//-----------------------------------------------------------------------------

void Network::convertUnits()
{
    units.setUnits(options);
    for (Node* node : nodes) node->convertUnits(this);
    for (Link* link : links) link->convertUnits(this);
    for (Control* control : controls) control->convertUnits(this);
}

//-----------------------------------------------------------------------------

bool Network::addElement(Element::ElementType element, int type, string name)
{

// Note: the caller of this function must insure that the network doesn't
//       already contain an element with the same name.

    try
    {
        if ( element == Element::NODE )
        {
            Node* node = Node::factory(type, name, memPool);
            node->index = nodes.size();
            nodeTable[node->name] = node;
            nodes.push_back(node);
        }

        else if ( element == Element::LINK )
        {
            Link* link = Link::factory (type, name, memPool);
            link->index = links.size();
            linkTable[link->name] = link;
            links.push_back(link);
        }

        else if ( element == Element::PATTERN )
        {
            Pattern* pattern = Pattern::factory(type, name, memPool);
            pattern->index = patterns.size();
            patternTable[pattern->name] = pattern;
            patterns.push_back(pattern);
        }

        else if ( element == Element::CURVE )
        {
            Curve* curve = new(memPool->alloc(sizeof(Curve))) Curve(name);
            curve->index = curves.size();
            curveTable[curve->name] = curve;
            curves.push_back(curve);
        }

        else if ( element == Element::CONTROL )
        {
            Control* control = new(memPool->alloc(sizeof(Control))) Control(type, name);
            control->index = controls.size();
            controlTable[control->name] = control;
            controls.push_back(control);
        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}

//-----------------------------------------------------------------------------

bool Network::createHeadLossModel()
{
    if ( headLossModel ) delete headLossModel;
    headLossModel = HeadLossModel::factory(
            option(Options::HEADLOSS_MODEL), option(Options::KIN_VISCOSITY));
    if ( headLossModel == nullptr )
    {
        throw SystemError(SystemError::HEADLOSS_MODEL_NOT_OPENED);
    }
	return true;
}

//-----------------------------------------------------------------------------

bool Network::createDemandModel()
{
    if ( demandModel ) delete demandModel;
    demandModel = DemandModel::factory(
            option(Options::DEMAND_MODEL), option(Options::PRESSURE_EXPONENT));
    if ( demandModel == nullptr )
    {
        throw SystemError(SystemError::DEMAND_MODEL_NOT_OPENED);
    }
	return true;
}

//-----------------------------------------------------------------------------

bool Network::createLeakageModel()
{
    if ( leakageModel ) delete leakageModel;
    if (option(Options::LEAKAGE_MODEL) == "NONE")
    {
        leakageModel = nullptr;
        return true;
    }
    leakageModel = LeakageModel::factory(option(Options::LEAKAGE_MODEL),
                                         ucf(Units::LENGTH),
                                         ucf(Units::FLOW));
    if ( leakageModel == nullptr )
    {
        throw SystemError(SystemError::LEAKAGE_MODEL_NOT_OPENED);
    }
    return true;
}

//-----------------------------------------------------------------------------

bool Network::createQualModel()
{
    if ( qualModel ) delete qualModel;
    if (option(Options::QUAL_MODEL) == "NONE")
    {
        qualModel = nullptr;
        return true;
    }
    qualModel = QualModel::factory(option(Options::QUAL_MODEL));
    if ( qualModel == nullptr )
    {
        throw SystemError(SystemError::QUALITY_MODEL_NOT_OPENED);
    }
	return true;
}

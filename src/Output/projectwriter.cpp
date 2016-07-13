/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "projectwriter.h"
#include "Core/network.h"
#include "Core/constants.h"
#include "Core/error.h"
#include "Elements/junction.h"
#include "Elements/reservoir.h"
#include "Elements/tank.h"
#include "Elements/pipe.h"
#include "Elements/pump.h"
#include "Elements/valve.h"
#include "Elements/pattern.h"
#include "Elements/curve.h"
#include "Elements/pattern.h"
#include "Elements/control.h"
#include "Elements/emitter.h"
#include "Elements/qualsource.h"
#include "Utilities/utilities.h"

#include <cmath>
#include <iomanip>
using namespace std;

//-----------------------------------------------------------------------------

ProjectWriter::ProjectWriter(): network(0)
{}

ProjectWriter::~ProjectWriter()
{}

//-----------------------------------------------------------------------------

//  Write the network's data base to a file using the EPANET INP format

int ProjectWriter::writeFile(const char* fname, Network* nw)
{
    if (nw == 0) return 0;
    network = nw;

    fout.open(fname, ios::out);
    if (!fout.is_open()) return FileError::CANNOT_OPEN_INPUT_FILE;

    writeTitle();
    writeJunctions();
    writeReservoirs();
    writeTanks();
    writePipes();
    writePumps();
    writeValves();
    writeDemands();
    writeEmitters();
    writeStatus();
    writePatterns();
    writeCurves();
    writeControls();
    writeEnergy();
    writeQuality();
    writeSources();
    writeMixing();
    writeReactions();
    writeOptions();
    writeTimes();
    writeReport();
    writeTags();
    writeCoords();
    writeAuxData();
    fout.close();
    return 0;
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeTitle()
{
    fout << "[TITLE]\n";
    network->writeTitle(fout);
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeJunctions()
{
    fout << "\n[JUNCTIONS]\n";
    for (Node* node : network->nodes)
    {
        if ( node->type() == Node::JUNCTION )
        {
            Junction* junc = static_cast<Junction*>(node);
            fout << left << setw(16) << node->name << " ";
            fout << setw(12) << fixed << setprecision(4);
            fout << node->elev*network->ucf(Units::LENGTH);

            if (network->option(Options::DEMAND_MODEL) != "FIXED" )
            {
                fout << "*     *    "; //Blank fields for primary demand and pattern
                double pUcf = network->ucf(Units::PRESSURE);
                fout << setw(12) << fixed << setprecision(4);
                fout << junc->pMin * pUcf;
                fout << setw(12) << fixed << setprecision(4);
                fout << junc->pFull * pUcf;
            }
            fout <<  "\n";
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeReservoirs()
{
    fout << "\n[RESERVOIRS]\n";
    for (Node* node : network->nodes)
    {
        if ( node->type() == Node::RESERVOIR )
        {
            Reservoir* resv = static_cast<Reservoir*>(node);
            fout << left << setw(16) << node->name << " ";
            fout << setw(12) << fixed << setprecision(4);
            fout << node->elev*network->ucf(Units::LENGTH);
            if ( resv->headPattern )
            {
                fout << resv->headPattern->name;
            }
            fout << "\n";
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeTanks()
{
    fout << "\n[TANKS]\n";
    for (Node* node : network->nodes)
    {
        if ( node->type() == Node::TANK )
        {
            Tank* tank = static_cast<Tank*>(node);
            double ucfLength = network->ucf(Units::LENGTH);
            fout << left << setw(16) << node->name << " ";
            fout << setw(12) << fixed << setprecision(4) << node->elev * ucfLength;
            fout << setw(12) << (tank->initHead - node->elev) * ucfLength;
            fout << setw(12) << (tank->minHead - node->elev) * ucfLength;
            fout << setw(12) << (tank->maxHead - node->elev) * ucfLength;
            fout << setw(12) << tank->diameter * ucfLength;
            fout << setw(12) << tank->minVolume * network->ucf(Units::VOLUME);
            if ( tank->volCurve ) fout << tank->volCurve->name;
            fout << "\n";
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writePipes()
{
    fout << "\n[PIPES]\n";
    for (Link* link : network->links)
    {
        if ( link->type() == Link::PIPE )
        {
            Pipe* pipe = static_cast<Pipe*>(link);
            fout << left << setw(16) << link->name << " ";
            fout << setw(16) << link->fromNode->name;
            fout << setw(16) << link->toNode->name;
            fout << setw(12) << fixed << setprecision(4);
            fout << pipe->length * network->ucf(Units::LENGTH);
            fout << setw(12) << pipe->diameter * network->ucf(Units::DIAMETER);
            double r = pipe->roughness;
            if ( network->option(Options::HEADLOSS_MODEL ) == "D-W")
            {
                r = r * network->ucf(Units::LENGTH) * 1000.0;
            }
            fout << setw(12) << r;
            fout << setw(12) << pipe->lossCoeff;
            if (pipe->hasCheckValve) fout << "CV\n";
            else if ( link->initStatus == Link::LINK_CLOSED ) fout << "CLOSED\n";
            else fout << "\n";
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writePumps()
{
    fout << "\n[PUMPS]\n";
    for (Link* link : network->links)
    {
        if ( link->type() == Link::PUMP )
        {
            Pump* pump = static_cast<Pump*>(link);
            fout << left << setw(16) << link->name << " ";
            fout << setw(16) << link->fromNode->name;
            fout << setw(16) << link->toNode->name;
            fout << fixed << setprecision(4);

            if ( pump->pumpCurve.horsepower > 0.0 )
            {
                fout << setw(8) << "POWER";
                fout << setw(12) << pump->pumpCurve.horsepower * network->ucf(Units::POWER);
            }

            if ( pump->pumpCurve.curveType != PumpCurve::NO_CURVE )
            {
                fout << setw(8) << "HEAD";
                fout << setw(16) << pump->pumpCurve.curve->name;
            }

            if ( pump->speed > 0.0 && pump->speed != 1.0 )
            {
                fout << setw(8) << "SPEED";
                fout << setw(8) << pump->speed;
            }

            if ( pump->speedPattern )
            {
                fout << setw(8) << "PATTERN";
                fout << setw(16) << pump->speedPattern->name;
            }
            fout << "\n";
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeValves()
{
    fout << "\n[VALVES]\n";
    for (Link* link : network->links)
    {
        if ( link->type() == Link::VALVE )
        {
            Valve* valve = static_cast<Valve*>(link);
            fout << left << setw(16) << link->name << " ";
            fout << setw(16) << link->fromNode->name;
            fout << setw(16) << link->toNode->name;
            fout << fixed << setprecision(4);
            fout << setw(12) << valve->diameter*network->ucf(Units::DIAMETER);
            fout << setw(8) << Valve::ValveTypeWords[(int)valve->valveType];

            if (valve->valveType == Valve::GPV)
            {
                fout << setw(16) << network->curve((int)link->initSetting)->name << "\n";
            }
            else
            {
                double cf = link->initSetting /
                            link->convertSetting(network, link->initSetting);
                fout << setw(12) << cf * link->initSetting << "\n";
            }
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeDemands()
{
    fout << "\n[DEMANDS]\n";
    for (Node* node : network->nodes)
    {
        if ( node->type() == Node::JUNCTION )
    	{
    	    Junction* junc = static_cast<Junction*>(node);
    	    fout << left;
    	    auto demand = junc->demands.begin();
    	    while ( demand != junc->demands.end() )
    	    {
    	        fout << setw(16) << node->name << " ";
    	        fout << setw(12) << fixed << setprecision(4);
    	        fout << demand->baseDemand * network->ucf(Units::FLOW);
    	        if (demand->timePattern != 0)
    	        {
    	            fout << setw(16) << demand->timePattern->name;
    	        }
    	        fout << "\n";
    	        ++demand;
    	    }
    	}
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeEmitters()
{
    fout << "\n[EMITTERS]\n";
    for (Node* node : network->nodes)
    {
        if ( node->type() == Node::JUNCTION )
        {
            Junction* junc = static_cast<Junction*>(node);
            Emitter* emitter = junc->emitter;
            if ( emitter )
            {
                fout << setw(16) << node->name << " ";
                double qUcf = network->ucf(Units::FLOW);
                double pUcf = network->ucf(Units::PRESSURE);
                fout << setw(12) << fixed << setprecision(4);
                fout << emitter->flowCoeff * qUcf * pow(pUcf, emitter->expon);
                fout << setw(12) << fixed << setprecision(4);
                fout << emitter->expon;
                if ( emitter->timePattern != 0 ) fout << setw(16) << emitter->timePattern->name;
                fout << "\n";
            }
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeLeakages()
{
    fout << "\n[LEAKAGES]\n";
    for (Link* link : network->links)
    {
        if ( link->type() == Link::PIPE )
        {
            Pipe* pipe = static_cast<Pipe*>(link);
            if ( pipe->leakCoeff1 > 0.0 )
            {
                fout << left << setw(16) << link->name;
                fout << setw(12) << fixed << setprecision(4);
                fout << pipe->leakCoeff1;
                fout << setw(12) << pipe->leakCoeff2;
                fout << "\n";
            }
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeStatus()
{
    fout << "\n[STATUS]\n";
    for (Link* link : network->links)
    {
    	if ( link->type() == Link::PUMP )
    	{
    	    if ( link->initSetting == 0 || link->initStatus == Link::LINK_CLOSED )
    	    {
    	        fout << left << setw(16) << link->name << "  CLOSED\n";
    	    }
    	}
    	else if ( link->type() == Link::VALVE )
    	{
    	    if ( link->initStatus == Link::LINK_OPEN || link->initStatus == Link::LINK_CLOSED )
    	    {
    	        fout << left << setw(16) << link->name << " ";
    	        if (link->initStatus == Link::LINK_OPEN) fout << "OPEN\n";
    	        else fout << "CLOSED\n";
    	    }
    	}
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writePatterns()
{
    fout << "\n[PATTERNS]\n";
    for (Pattern* pattern : network->patterns)
    {
    	if ( pattern->type == Pattern::FIXED_PATTERN )
    	{
            fout << left << setw(16) << pattern->name << " FIXED ";
            if ( pattern->timeInterval() > 0 ) fout << Utilities::getTime(pattern->timeInterval());
            int k = 0;
            int i = 0;
            int n = pattern->size();
            while ( i < n )
            {
                if ( k == 0 ) fout << "\n" << setw(16) << pattern->name << "  ";
                fout << setw(12) << fixed << setprecision(4) << pattern->factor(i);
                i++;
                k++;
                if ( k == 5 ) k = 0;
            }
        }
    	else if (pattern-> type == Pattern::VARIABLE_PATTERN )
    	{
    	    VariablePattern* vp = static_cast<VariablePattern*>(pattern);
    	    fout << left << setw(16) << pattern->name << " VARIABLE ";
    	    for (int i = 0; i < pattern->size(); i++)
    	    {
    	        fout << "\n" << setw(16) << pattern->name << "  ";
    	        fout << setw(12) << fixed << setprecision(4);
    	        fout << Utilities::getTime((int)vp->time(i)) << vp->factor(i) << "\n";
    	    }
    	}
        fout << "\n";
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeCurves()
{
    fout << "\n[CURVES]\n";
    for (Curve* curve : network->curves)
    {
        if (curve->curveType() != Curve::UNKNOWN)
        {
            fout << left << setw(16) << curve->name << "  " <<
                Curve::CurveTypeWords[curve->curveType()] << "\n";
        }
        for (int i = 0; i < curve->size(); i++)
        {
            fout << setw(16) << curve->name << "  ";
            fout << setw(12) << curve->x(i);
            fout << setw(12) << curve->y(i) << "\n";
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeControls()
{
    fout << "\n[CONTROLS]\n";
    for (Control* control : network->controls)
    {
        fout << control->toStr(network) << "\n";
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeEnergy()
{
    fout << "\n[ENERGY]\n";
    fout << network->options.energyOptionsToStr(network);
    for (Link* link : network->links)
    {
    	if ( link->type() == Link::PUMP )
    	{
    	    Pump* pump = static_cast<Pump*>(link);
    	    fout << left << fixed << setprecision(4);
    	    if ( pump->efficCurve )
    	    {
    	        fout << "PUMP  " << link->name << "  " << "EFFIC  ";
    	        fout << pump->efficCurve->name << "\n";
    	    }

    	    if ( pump->costPerKwh > 0.0 )
    	    {
    	        fout << "PUMP  " << link->name << "  " << "PRICE  " << pump->costPerKwh << "\n";
    	    }

    	    if ( pump->costPattern )
    	    {
    	        fout << "PUMP  " << link->name << "  " << "PATTERN  ";
    	        fout << pump->costPattern->name << "\n";
    	    }
    	}
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeQuality()
{
    fout << "\n[QUALITY]\n";
    for (Node* node : network->nodes)
    {
        if (node->initQual > 0.0)
        {
            fout << left << setw(16) << node->name << " ";
            fout << setw(12) << fixed << setprecision(4);
            fout << node->initQual * network->ucf(Units::CONCEN) << "\n";
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeSources()
{
    fout << "\n[SOURCES]\n";
    for (Node* node : network->nodes)
    {
        if ( node->qualSource && node->qualSource->base > 0.0)
        {
            fout << left << setw(16) << node->name << " ";
            fout << setw(12) << fixed << setprecision(4);
            fout << QualSource::SourceTypeWords[node->qualSource->type];
            fout << node->qualSource->base;
            if ( node->qualSource->pattern )
            {
                fout << node->qualSource->pattern->name;
            }
            fout << "\n";
        }
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeMixing()
{
    fout << "\n[MIXING]\n";
    for (Node* node : network->nodes)
    {
        if ( node->type() == Node::TANK )
    	{
    	    Tank* tank = static_cast<Tank*>(node);
    	    fout << left << setw(16) << node->name << " ";
    	    fout << setw(12) << fixed << setprecision(4);
    	    fout << TankMixModel::MixingModelWords[tank->mixingModel.type];
    	    fout << tank->mixingModel.fracMixed << "\n";
    	}
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeReactions()
{
    fout << "\n[REACTIONS]\n";
    fout << network->options.reactOptionsToStr();
    double defBulkCoeff = network->option(Options::BULK_COEFF);
    double defWallCoeff = network->option(Options::WALL_COEFF);

    for (Link* link : network->links)
    {
     	if ( link->type() == Link::PIPE )
    	{
    	    Pipe* pipe = static_cast<Pipe*>(link);
            fout << left << fixed << setprecision(4);
            if ( pipe->bulkCoeff != defBulkCoeff )
            {
                fout << "BULK      ";
                fout << setw(16) << link->name << " ";
                fout << pipe->bulkCoeff << "\n";
            }
            if ( pipe->wallCoeff != defWallCoeff )
            {
                fout << "WALL      ";
                fout << setw(16) << link->name << " ";
                fout << pipe->wallCoeff << "\n";
            }
    	}
    }

    for (Node* node : network->nodes)
    {
        if ( node->type() == Node::TANK )
    	{
    	    Tank* tank = static_cast<Tank*>(node);
    	    if ( tank->bulkCoeff != defBulkCoeff )
    	    {
    	        fout << "TANK      ";
    	        fout << setw(16) << node->name << " ";
    	        fout << tank->bulkCoeff << "\n";
    	    }
    	}
    }
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeOptions()
{
    fout << "\n[OPTIONS]\n";
    fout << network->options.hydOptionsToStr();
    fout << "\n";
    fout << network->options.demandOptionsToStr();
    fout << "\n";
    fout << network->options.qualOptionsToStr();
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeTimes()
{
    fout << "\n[TIMES]\n";
    fout << network->options.timeOptionsToStr();
}

//-----------------------------------------------------------------------------

void ProjectWriter::writeReport()
{
    fout << "\n[REPORT]\n";
    fout << network->options.reportOptionsToStr();
}

void ProjectWriter::writeTags()
{
}

void ProjectWriter::writeCoords()
{
}

void ProjectWriter::writeAuxData()
{
}

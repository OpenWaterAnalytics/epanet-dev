/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "reportwriter.h"
#include "outputfile.h"
#include "Core/network.h"
#include "Core/error.h"
#include "Elements/node.h"
#include "Elements/link.h"
#include "Utilities/utilities.h"

#include <cmath>
#include <iomanip>
using namespace std;

//-----------------------------------------------------------------------------

static const string statusTxt[] = {"CLOSED", "OPEN", "ACTIVE", "CLOSED"};
static const int width = 12;
static const int precis = 3;

//-----------------------------------------------------------------------------

ReportWriter::ReportWriter(ofstream& ofs, Network* nw) : sout(ofs), network(nw)
{
}

//-----------------------------------------------------------------------------

ReportWriter::~ReportWriter()
{}

//-----------------------------------------------------------------------------

void ReportWriter::writeHeading()
{
    sout << left;
    sout << "\n  ******************************************************************";
    sout << "\n  *                           E P A N E T                          *";
    sout << "\n  *                   Hydraulic and Water Quality                  *";
    sout << "\n  *                   Analysis for Pipe Networks                   *";
    sout << "\n  *                         Version 3.0.000                        *";
    sout << "\n  ******************************************************************\n";
}

//-----------------------------------------------------------------------------

void ReportWriter::writeSummary(string inpFileName)
{
    if ( network == nullptr ) return;
    network->writeTitle(sout);
    if ( !network->option(Options::REPORT_SUMMARY) ) return;

    sout << "\n  Input Data File ............... "
         << Utilities::getFileName(inpFileName);

    int nJuncs = 0;
    int nResvs = 0;
    int nTanks = 0;
    int nEmitters = 0;
    int nSources = 0;
    for (Node* node : network->nodes)
    {
        switch (node->type())
        {
        case Node::JUNCTION : nJuncs++; break;
        case Node::RESERVOIR: nResvs++; break;
        case Node::TANK:      nTanks++; break;
        }
        if ( node->hasEmitter() ) nEmitters++;
        if (node->qualSource) nSources++;
    }

    sout << "\n  Number of Junctions ........... " << nJuncs;
    sout << "\n  Number of Reservoirs .......... " << nResvs;
    sout << "\n  Number of Tanks ............... " << nTanks;

    int nPipes = 0;
    int nPumps = 0;
    int nValves = 0;
    for (Link* link : network->links)
    {
        switch (link->type())
        {
        case Link::PIPE:  nPipes++;  break;
        case Link::PUMP:  nPumps++;  break;
        case Link::VALVE: nValves++; break;
        }
    }

    sout << "\n  Number of Pipes ............... " << nPipes;
    sout << "\n  Number of Pumps ............... " << nPumps;
    sout << "\n  Number of Valves .............. " << nValves;

    sout << "\n  Head Loss Model ............... " << network->option(Options::HEADLOSS_MODEL);
    sout << "\n  Leakage Model ................. " << network->option(Options::LEAKAGE_MODEL);
    sout << "\n  Demand Model .................. " << network->option(Options::DEMAND_MODEL);
    sout << "\n  Demand Multiplier ............. " << network->option(Options::DEMAND_MULTIPLIER);
    sout << "\n  Number of Emitters ............ " << nEmitters;
    sout << "\n  Head Tolerance ................ " << network->option(Options::HEAD_TOLERANCE);
    sout << "\n  Flow Tolerance ................ " << network->option(Options::FLOW_TOLERANCE);
    sout << "\n  Flow Change Limit ............. " << network->option(Options::FLOW_CHANGE_LIMIT);

    sout << "\n  Quality Model ................. " << network->option(Options::QUAL_MODEL);
    if ( network->option(Options::QUAL_TYPE) == Options::TRACE )
        sout << " Node " << network->option(Options::TRACE_NODE_NAME);
    if ( network->option(Options::QUAL_TYPE) == Options::CHEM )
        sout << "\n  Quality Constituent ........... " << network->option(Options::QUAL_NAME);
    if ( network->option(Options::QUAL_TYPE) != Options::NOQUAL )
        sout << "\n  Number of Sources ............. " << nSources;

    sout << "\n  Hydraulic Time Step ........... " << network->option(Options::HYD_STEP) / 60 << " minutes";
    if ( network->option(Options::QUAL_TYPE) != Options::NOQUAL )
    sout << "\n  Quality Time Step ............. " << network->option(Options::QUAL_STEP) << " seconds";
    sout << "\n  Report Time Step .............. " << network->option(Options::REPORT_STEP) / 60 << " minutes";
    sout << "\n  Total Duration ................ " << network->option(Options::TOTAL_DURATION) / 3600 << " hours";
    sout << "\n";
}

//-----------------------------------------------------------------------------

//  Access the results saved to a binary file to write a formatted report a text file.

int  ReportWriter::writeReport(string inpFileName, OutputFile* outFile)
{
    // ... check if any output results exist
    if ( outFile->timePeriodCount == 0)
    {
        return FileError::NO_RESULTS_SAVED_TO_REPORT;
    }
    if ( !outFile->initReader() ) return FileError::CANNOT_OPEN_OUTPUT_FILE;

    // ... check if a separate report file was named in reporting options
    bool usingRptFile2 = false;
    string rptFileName2 = network->option(Options::RPT_FILE_NAME);
    if ( rptFileName2.length() > 0 )
    {
        // ... write report to this file
        sout.open(rptFileName2);
        if (!sout.is_open()) return FileError::CANNOT_OPEN_REPORT_FILE;
        usingRptFile2 = true;

        // ... write report heading and network summary
        //     (status log is only written to primary report file)
        writeHeading();
        writeSummary(inpFileName);
    }

    // ... otherwise write report to the primary report file
    else
    {
        if ( !sout.is_open() ) return FileError::CANNOT_WRITE_TO_REPORT_FILE;
        sout << network->msgLog.str();
        network->msgLog.str("");
    }
    writeEnergyResults(outFile);
    writeSavedResults(outFile);

    // ... close the secondary report file if used
    if ( usingRptFile2 ) sout.close();
    return 0;
}

//-----------------------------------------------------------------------------

//  Write the current set of results computed at time t to a report file.

void ReportWriter::writeResults(int t)
{
    if (!network) return;
    string theTime = Utilities::getTime(t);
    double lcf = network->ucf(Units::LENGTH);
    double pcf = network->ucf(Units::PRESSURE);
    double qcf = network->ucf(Units::FLOW);
    double ccf = network->ucf(Units::CONCEN);
    double outflow;
    if (network->option(Options::REPORT_NODES))
    {
        float nodeResults[NumNodeVars];
        sout << left;
        sout << endl << endl << "  Node Results at " << theTime << " hrs" << endl;
        writeNodeHeader();
        for (Node* node : network->nodes)
        {
            nodeResults[0] = (float)(node->head * lcf);
            nodeResults[1] = (float)((node->head - node->elev) * pcf);
            nodeResults[2] = (float)(node->actualDemand * qcf);
            nodeResults[3] = (float)((node->fullDemand - node->actualDemand) * qcf);
            outflow = node->outflow * qcf;
            if ( node->type() != Node::JUNCTION ) outflow = -outflow;
            nodeResults[4] = (float)(outflow);
            nodeResults[5] = (float)(node->quality*ccf);
            writeNodeResults(node, nodeResults);
        }
    }
    if (network->option(Options::REPORT_LINKS))
    {
        float linkResults[NumLinkVars];
        sout << left;
        sout << endl << endl << "  Link Results at " << theTime << " hrs" << endl;
        writeLinkHeader();
        for (Link* link : network->links)
        {
            linkResults[0] = (float)(link->flow * qcf);
            linkResults[1] = (float)(link->leakage * qcf);
            linkResults[2] = (float)(link->getVelocity() * lcf);
            double uhl = link->getUnitHeadLoss();
            if ( link->type() != Link::PIPE ) uhl *= lcf;
            linkResults[3] = (float)uhl;
            linkResults[4] = (float)link->status;
            writeLinkResults(link, linkResults);
        }
    }
}

//-----------------------------------------------------------------------------

//  Write the energy usage results saved in a binary output file to the report file.

void ReportWriter::writeEnergyResults(OutputFile* outFile)
{
    if ( !network->option(Options::REPORT_ENERGY) ) return;
    int nPumps = outFile->pumpCount;
    if ( nPumps == 0 ) return;

    writeEnergyHeader();
    outFile->seekEnergyOffset();

    int pumpIndex;
    float totalCost = 0.0;
    for (int p = 0; p < nPumps; p++)
    {
        outFile->readEnergyResults(&pumpIndex);
        Link* link = network->link(pumpIndex);
        writePumpResults(link, outFile->pumpResults);
        totalCost += outFile->pumpResults[5];
    }
    sout << left;
    string line(96, '-');
    sout << "  " << line << endl;

    float demandCharge;
    outFile->readEnergyDemandCharge(&demandCharge);
    sout << setw(86) << "  Demand Charge:";
    sout << right << showpoint << fixed;
    writeNumber(demandCharge, 12, 2);
    sout << left << endl;
    sout << setw(86) << "  Total Cost:";
    sout << right;
    writeNumber(totalCost + demandCharge, 12, 2);
    sout << left << endl;
}

//-----------------------------------------------------------------------------

void ReportWriter::writeEnergyHeader()
{
    string line(96, '-');
    sout << left;
    sout << endl << endl << endl << "  Energy Usage:" << endl;
    sout << "  " << line << endl;

    sout << right;
    sout << setw(26) << " ";
    sout << setw(12) << "% Time" << setw(12) << "Avg. %" << setw(12) << "Kw-hr";
    sout << setw(12) << "Avg." << setw(12) << "Peak" << setw(12) << "Cost" << endl;

    sout << left;
    sout << setw(26) << "  Pump";
    sout << right;
    sout << setw(12) << "Online" << setw(12) << "Effic.";
    string volume;
    if ( network->option(Options::UNIT_SYSTEM) == Options::US ) volume = "Mgal";
    else volume = "m3";
    sout << setw(12) << volume;
    sout << setw(12) << "Kwatts" << setw(12) << "Kwatts" << setw(12) << "/day" << endl;

    sout << left;
    sout << "  " << line << endl;
}

//-----------------------------------------------------------------------------

//  Write the energy usage results for a specific pump to the formatted report file.

void ReportWriter::writePumpResults(Link* link, float* x)
{
    sout << left;
    sout << "  " << setw(24) << link->name;
    sout << right << showpoint;
    for (int i = 0; i < NumPumpVars; i++) writeNumber(x[i], 12, 2);
    sout << endl;
}

//-----------------------------------------------------------------------------

//  Write saved results to the formatted report file at each report time period.

void ReportWriter::writeSavedResults(OutputFile* outFile)
{
    int nPeriods = outFile->timePeriodCount;
    int reportStep = outFile->reportStep;
    outFile->seekNetworkOffset();
    int t = outFile->reportStart;
    for (int i = 1; i <= nPeriods; i++)
    {
        string theTime = Utilities::getTime(t);

        if (network->option(Options::REPORT_NODES))
        {
            sout << left;
            sout << endl << endl << "  Node Results at " << theTime << " hrs" << endl;
            writeNodeHeader();
            for (Node* node : network->nodes)
            {
                outFile->readNodeResults();
                writeNodeResults(node, outFile->nodeResults);
            }
        }
        else outFile->skipNodeResults();

        if (network->option(Options::REPORT_LINKS))
        {
            sout << left;
            sout << endl << endl << "  Link Results at " << theTime << " hrs" << endl;
            writeLinkHeader();
            for (Link* link : network->links)
            {
                outFile->readLinkResults();
                writeLinkResults(link, outFile->linkResults);
            }
        }
        else outFile->skipLinkResults();

        t += reportStep;
    }
}

//-----------------------------------------------------------------------------

void ReportWriter::writeNodeResults(Node* node, float* x)
{
    sout << left;
    sout << "  " << setw(24) << node->name;
    sout << right << fixed << showpoint;
    for (int i = 0; i < NumNodeVars-1; i++) writeNumber(x[i], width, precis);
    if ( network->option(Options::QUAL_TYPE) != Options::NOQUAL )
    {
        writeNumber(x[NumNodeVars-1], width, precis);
    }
    sout << endl;
    sout << left;
}

//-----------------------------------------------------------------------------

void ReportWriter::writeNodeHeader()
{
    bool hasQual = network->option(Options::QUAL_TYPE) != Options::NOQUAL;
    sout << left;
    string s1(84, '-');
    string s2 = "";
    if ( hasQual ) s2 = "------------";
    sout << "  " << s1 << s2 << endl;

    sout << setw(26) << " ";
    sout << "        Head    Pressure      Demand     Deficit     Outflow";
    if ( hasQual )
    {
        sout << right;
        sout << setw(12) << network->option(Options::QUAL_NAME);
    }
    sout << endl;

    sout << left;
    sout << setw(26) << "  Node";
    sout << right;
    sout << setw(12) << network->getUnits(Units::LENGTH);
    sout << setw(12) << network->getUnits(Units::PRESSURE);
    sout << setw(12) << network->getUnits(Units::FLOW);
    sout << setw(12) << network->getUnits(Units::FLOW);
    sout << setw(12) << network->getUnits(Units::FLOW);
    if ( hasQual ) sout << setw(12) << network->option(Options::QUAL_UNITS_NAME);
    sout << endl;

    sout << left;
    sout << "  " << s1 << s2 << endl;
}

//-----------------------------------------------------------------------------

void ReportWriter::writeLinkResults(Link* link, float* x)
{
    sout << left;
    sout << "  " << setw(24) << link->name;
    sout << right << fixed << showpoint;

    writeNumber(x[0], width, precis);
    writeNumber(x[1], width, precis);
    writeNumber(x[2], width, precis);
    writeNumber(x[3], width, precis);

    sout << setw(12) << statusTxt[(int)x[4]];
    sout << left;
    if ( link->type() != Link::PIPE )
    {
        sout << "/" << link->typeStr();
    }
    sout << endl;
}

//-----------------------------------------------------------------------------

void ReportWriter::writeLinkHeader()
{
    sout << left;
    string s1(72, '-');
    sout << "  " << s1 << endl;
    sout << setw(26) << " ";
    sout << "   Flow Rate     Leakage    Velocity   Head Loss      Status" << endl;

    sout << setw(26) << "  Link";
    sout << right;
    sout << setw(12) << network->getUnits(Units::FLOW);
    sout << setw(12) << network->getUnits(Units::FLOW);
    sout << setw(12) << network->getUnits(Units::VELOCITY);
    sout << setw(12) << network->getUnits(Units::HEADLOSS) << endl;

    sout << left;
    sout << "  " << s1 << endl;
}

//-----------------------------------------------------------------------------

//  Write a number to the report file stream (sout) in either fixed or
//  scientific format depending on its magnitude.

void ReportWriter::writeNumber(float x, int w, int p)
{
    if ( x < 1.0e-4 && x > -1.e-4 ) x = 0.0;
    double absX = abs(x);
    if ( absX > 0.0 && (absX > 1.0e5 || absX < 1.0e-5) )
    {
        sout << scientific;
    }
    else sout << fixed;
    sout << setw(w) << setprecision(p) << x;
    sout << fixed;
}

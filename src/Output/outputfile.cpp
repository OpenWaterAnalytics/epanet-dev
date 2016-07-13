/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "outputfile.h"
#include "Core/network.h"
#include "Core/constants.h"
#include "Core/error.h"
#include "Elements/node.h"
#include "Elements/link.h"
#include "Elements/pipe.h"
#include "Elements/pump.h"
#include "Elements/valve.h"
#include "Elements/qualsource.h"

#include <cmath>
using namespace std;

static int findPumpCount(Network* nw);

//-----------------------------------------------------------------------------

OutputFile::OutputFile():
    fname(""),
    network(nullptr),
    nodeCount(0),
    linkCount(0),
    pumpCount(0),
    timePeriodCount(0),
    reportStart(0),
    reportStep(0),
    energyResultsOffset(0),
    networkResultsOffset(0)
{
}

//-----------------------------------------------------------------------------

OutputFile::~OutputFile()
{
    close();
}

//-----------------------------------------------------------------------------

int OutputFile::open(const string fileName, Network* nw)
{
    close();
    fwriter.open(fileName.c_str(), ios::out | ios::binary | ios::trunc);
    if ( !fwriter.is_open() ) return FileError::CANNOT_OPEN_OUTPUT_FILE;
    fname = fileName;
    network = nw;
    return 0;
}

//-----------------------------------------------------------------------------

void OutputFile::close()
{
    fwriter.close();
    freader.close();
    network = 0;
}

//-----------------------------------------------------------------------------

int OutputFile::initWriter()
{
    // ... return if output file not previously opened
    if ( !fwriter.is_open() || !network ) return 0;

    // ... re-open the output file
    fwriter.close();
    freader.close();
    fwriter.open(fname.c_str(), ios::out | ios::binary | ios::trunc);
    if ( !fwriter.is_open() ) return FileError::CANNOT_OPEN_OUTPUT_FILE;

    // ... retrieve element counts
    nodeCount = network->count(Element::NODE);
    linkCount = network->count(Element::LINK);
    pumpCount = findPumpCount(network);

    // ... retrieve reporting time steps
    timePeriodCount = 0;
    reportStart = network->option(Options::REPORT_START);
    reportStep = network->option(Options::REPORT_STEP);

    // ... compute byte offsets for where energy results and network results begin
    energyResultsOffset = NumSysVars * IntSize;
    networkResultsOffset = energyResultsOffset + pumpCount *
                           (IntSize + NumPumpVars * FloatSize) + FloatSize;

    // ... write system info to the output file
    int sysBuf[NumSysVars];
    sysBuf[0] = MAGICNUMBER;
    sysBuf[1] = VERSION;
    sysBuf[2] = 0;                     // reserved for error code
    sysBuf[3] = 0;                     // reserved for warning flag
    sysBuf[4] = energyResultsOffset;
    sysBuf[5] = networkResultsOffset;
    sysBuf[6] = nodeCount;
    sysBuf[7] = linkCount;
    sysBuf[8] = pumpCount;
    sysBuf[9] = network->option(Options::QUAL_TYPE);
    sysBuf[10] = network->option(Options::TRACE_NODE);
    sysBuf[11] = network->option(Options::UNIT_SYSTEM);
    sysBuf[12] = network->option(Options::FLOW_UNITS);
    sysBuf[13] = network->option(Options::PRESSURE_UNITS);
    sysBuf[14] = network->option(Options::QUAL_UNITS);
    sysBuf[15] = network->option(Options::REPORT_STATISTIC);
    sysBuf[16] = reportStart;
    sysBuf[17] = reportStep;
    sysBuf[18] = NumNodeVars;
    sysBuf[19] = NumLinkVars;
    sysBuf[20] = NumPumpVars;
    fwriter.write((char *)sysBuf, sizeof(sysBuf));
    if ( fwriter.fail() ) return FileError::CANNOT_WRITE_TO_OUTPUT_FILE;

    // ... position the file to where network results begins
    fwriter.seekp(networkResultsOffset);
    return 0;
}

//-----------------------------------------------------------------------------

int OutputFile::writeEnergyResults(double totalHrs, double peakKwatts)
{
    // ... position output file to start of energy results
    if ( !fwriter.is_open() || !network ) return 0;
    fwriter.seekp(energyResultsOffset);

    // ... adjust total hrs online for single period analysis
    if ( totalHrs == 0.0 ) totalHrs = 24.0;

    // ... scan network links for pumps
    int index = -1;
    for (Link* link: network->links)
    {
        // ... skip non-pump links
        index++;
        if ( link->type() != Link::PUMP ) continue;
        Pump* p = static_cast<Pump *>(link);

        // ... percent of time online
        double t = p->pumpEnergy.hrsOnLine;
        pumpResults[0] = (float)(t / totalHrs * 100.0);

        // ... avg. percent efficiency
        pumpResults[1] = (float)(p->pumpEnergy.efficiency);

        // ... avg. kw-hr per mil. gal (or per cubic meter)
        double cf;
        if ( network->option(Options::UNIT_SYSTEM) == Options::SI )
        {
            cf = 1000.0/LPSperCFS/3600.0;
        }
        else cf = 1.0e6/GPMperCFS/60.0;
        pumpResults[2] = (float)(p->pumpEnergy.kwHrsPerCFS * cf);

        // ... avg. kwatts
        pumpResults[3] = (float)(p->pumpEnergy.kwHrs);

        // ... peak kwatts
        pumpResults[4] = (float)(p->pumpEnergy.maxKwatts);

        // ... total cost per day
        pumpResults[5] = (float)(p->pumpEnergy.totalCost * 24.0 / totalHrs);

        // ... write link index and energy results to file
        fwriter.write((char *)&index, IntSize);
        fwriter.write((char *)pumpResults, sizeof(pumpResults));
    }

    // ... save demand (peaking factor) charge
    float demandCharge = (float)(peakKwatts * network->option(Options::PEAKING_CHARGE));
    fwriter.write((char *)&demandCharge, sizeof(demandCharge));

    // ... save number of periods simulated
    fwriter.seekp(2 * IntSize);
    fwriter.write((char *)&timePeriodCount, IntSize);
    if ( fwriter.fail() ) return FileError::CANNOT_WRITE_TO_OUTPUT_FILE;
    return 0;
}

//-----------------------------------------------------------------------------

int OutputFile::writeNetworkResults()
{
    if ( !fwriter.is_open() || !network ) return 0;
    timePeriodCount++;
    writeNodeResults();
    writeLinkResults();
    if ( fwriter.fail() ) return FileError::CANNOT_WRITE_TO_OUTPUT_FILE;
    return 0;
}

//-----------------------------------------------------------------------------

int findPumpCount(Network* nw)
{
    int count = 0;
    for (Link* link: nw->links)
    {
        if ( link->type() == Link::PUMP ) count++;
    }
    return count;
}

//-----------------------------------------------------------------------------

void OutputFile::writeNodeResults()
{
    if ( fwriter.fail() ) return;

    // ... units conversion factors
    double lcf = network->ucf(Units::LENGTH);
    double pcf = network->ucf(Units::PRESSURE);
    double qcf = network->ucf(Units::FLOW);
    double ccf = network->ucf(Units::CONCEN);
    double outflow;
    double quality;

    // ... results for each node
    for (Node* node: network->nodes)
    {
        // ... head, pressure, & actual demand
        nodeResults[0] = (float)(node->head*lcf);
        nodeResults[1] = (float)((node->head - node->elev)*pcf);
        nodeResults[2] = (float)(node->actualDemand*qcf);

        // ... demand deficit
        nodeResults[3] =
            (float)((node->fullDemand - node->actualDemand)*qcf);

        // ... total external outflow (reverse sign for tanks & reservoirs)
        outflow = node->outflow;
        if ( node->type() != Node::JUNCTION ) outflow = -outflow;
        nodeResults[4] = (float)(outflow*qcf);

        // ... use source-ammended quality for WQ source nodes
        if ( node->qualSource ) quality = node->qualSource->quality;
        else                    quality = node->quality;
        nodeResults[5] = (float)(quality*ccf);

        fwriter.write((char *)nodeResults, sizeof(nodeResults));
    }
}

//-----------------------------------------------------------------------------

void OutputFile::writeLinkResults()
{
    if ( fwriter.fail() ) return;

    // ... units conversion factors
    double lcf = network->ucf(Units::LENGTH);
    double qcf = network->ucf(Units::FLOW);
    double hloss;

    // ... results for each link
    for (Link* link: network->links)
    {
        linkResults[0] = (float)(link->flow*qcf);                    //flow
        linkResults[1] = (float)(link->leakage*qcf);                 //leakage
        linkResults[2] = (float)(link->getVelocity()*lcf);           //velocity
        hloss = link->getUnitHeadLoss();
        if ( link->type() != Link::PIPE ) hloss *= lcf;
        linkResults[3] = (float)(hloss);                             //head loss
        linkResults[4] = (float)link->status;                        //status
        linkResults[5] = (float)link->getSetting(network);           //setting
        linkResults[6] = (float)(link->quality*FT3perL);             //quality

        fwriter.write((char *)linkResults, sizeof(linkResults));
    }
}

//-----------------------------------------------------------------------------

//// The following set of functions reads results back from the binary output
//// file for use when constructing a report written to a text file.

int OutputFile::initReader()
{
    fwriter.close();
    freader.close();
    freader.open(fname.c_str(), ios::in | ios::binary);
    if ( !freader.is_open() ) return 0;
    return 1;
}

void OutputFile::seekEnergyOffset()
{
    freader.seekg(energyResultsOffset);
}

void OutputFile::readEnergyResults(int* pumpIndex)
{
    freader.read((char *)pumpIndex, IntSize);
    freader.read((char *)pumpResults, sizeof(pumpResults));
}

void OutputFile::readEnergyDemandCharge(float* demandCharge)
{
    freader.read((char *)demandCharge, FloatSize);
}

void OutputFile::seekNetworkOffset()
{
    freader.seekg(networkResultsOffset);
}

void OutputFile::readNodeResults()
{
    freader.read((char *)nodeResults, sizeof(nodeResults));
}

void OutputFile::readLinkResults()
{
    freader.read((char *)linkResults, sizeof(linkResults));
}

void OutputFile::skipNodeResults()
{
    freader.seekg(nodeCount*sizeof(nodeResults), ios::cur);
}

void OutputFile::skipLinkResults()
{
    freader.seekg(linkCount*sizeof(linkResults), ios::cur);
}

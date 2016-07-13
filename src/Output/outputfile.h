/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file outputfiler.h
//! \brief Description of the OutputFile class.

#ifndef OUTPUTFILE_H_
#define OUTPUTFILE_H_

#include <fstream>
#include <string>

class Network;
class ReportWriter;

const    int   IntSize = sizeof(int);
const    int   FloatSize = sizeof(float);
const    int   NumSysVars = 21;
const    int   NumNodeVars = 6;
const    int   NumLinkVars = 7;
const    int   NumPumpVars = 6;

//! \class OutputFile
//! \brief Manages the writing and reading of analysis results to a binary file.

class OutputFile
{
  public:
    OutputFile();
    ~OutputFile();

    int    open(const std::string fileName, Network* nw);
    void   close();

    int    initWriter();
    int    writeEnergyResults(double totalHrs, double peakKwatts);
    int    writeNetworkResults();

    int    initReader();
    void   seekEnergyOffset();
    void   readEnergyResults(int* pumpIndex);
    void   readEnergyDemandCharge(float* demandCharge);
    void   seekNetworkOffset();
    void   readNodeResults();
    void   readLinkResults();
    void   skipNodeResults();
    void   skipLinkResults();

    friend ReportWriter;

  private:
    std::string   fname;                    //!< name of binary output file
    std::ofstream fwriter;                  //!< output file stream.
    std::ifstream freader;                  //!< file input stream
    Network*      network;                  //!< associated network
    int           nodeCount;                //!< number of network nodes
    int           linkCount;                //!< number of network links
    int           pumpCount;                //!< number of pump links
    int           timePeriodCount;          //!< number of time periods written
    int           reportStart;              //!< time when reporting starts (sec)
    int           reportStep;               //!< time between reporting periods (sec)
    int           energyResultsOffset;      //!< offset for pump energy results
    int           networkResultsOffset;     //!< offset for extended period results
    float         nodeResults[NumNodeVars]; //!< array of node results
    float         linkResults[NumLinkVars]; //!< array of link results
    float         pumpResults[NumPumpVars]; //!< array of pump results
    void          writeNodeResults();
    void          writeLinkResults();
};

#endif

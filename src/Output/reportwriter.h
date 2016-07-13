/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file reportwriter.h
//! \brief Description of the ReportWriter class.

#ifndef REPORTWRITER_H_
#define REPORTWRITER_H_

#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>

class Network;
class Node;
class Link;
class OutputFile;

class ReportWriter
{
  public:
    ReportWriter(std::ofstream& ofs, Network* nw);
    ~ReportWriter();

    void writeHeading();
    void writeSummary(std::string inpFileName);
    void writeResults(int t);
    int  writeReport(std::string inpFileName, OutputFile* outFile);

  private:
    std::ofstream& sout;
    Network* network;

    void writeEnergyResults(OutputFile* outFile);
    void writeEnergyHeader();
    void writePumpResults(Link* link, float* x);
    void writeSavedResults(OutputFile* outFile);
    void writeLinkHeader();
    void writeLinkResults(Link* link, float* x);
    void writeNodeHeader();
    void writeNodeResults(Node* node, float* x);
    void writeNumber(float x, int width, int precis);
};

#endif

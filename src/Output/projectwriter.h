/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file projectwriter.h
//! \brief Describes the ProjectWriter class.

#ifndef INPUTWRITER_H_
#define INPUTWRITER_H_

#include <fstream>

class Network;

//! \class ProjectWriter
//! \brief The ProjectWriter class writes a project's data to file.

class ProjectWriter
{
  public:
    ProjectWriter();
    ~ProjectWriter();
    int writeFile(const char* fname, Network* nw);

  private:
    void writeTitle();
    void writeJunctions();
    void writeReservoirs();
    void writeTanks();
    void writePipes();
    void writePumps();
    void writeValves();
    void writeDemands();
    void writeEmitters();
    void writeLeakages();
    void writeStatus();
    void writePatterns();
    void writeCurves();
    void writeControls();
    void writeQuality();
    void writeSources();
    void writeMixing();
    void writeReactions();
    void writeEnergy();
    void writeTimes();
    void writeOptions();
    void writeReport();
    void writeTags();
    void writeCoords();
    void writeAuxData();
    Network* network;
    std::fstream fout;
};

#endif

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file project.h
//! \brief Describes EPANET's Project class.

//! \mainpage EPANET
//! EPANET performs extended period hydraulic and water quality analysis of
//! looped, pressurized piping networks. A network consists of pipes, pumps,
//! valves, junctions, storage tanks and reservoirs. EPANET tracks
//! the flow of water in each pipe, the pressure at each junction, the height
//! of water in each tank, and the concentration of a chemical species
//! throughout the network during a simulation period comprised of multiple
//! time steps. In addition to chemical species, water age and source tracing
//! can also be simulated.

#ifndef PROJECT_H_
#define PROJECT_H_

#include "Core/network.h"
#include "Core/hydengine.h"
#include "Core/qualengine.h"
#include "Output/outputfile.h"

#include <string>
#include <fstream>

namespace Epanet
{
    //!
    //! \class Project
    //! \brief Encapsulates a pipe network and its simulation engines.
    //!
    //! A project contains a description of the pipe network being analyzed
    //! and the engines and methods used to carry out the analysis. All
    //! methods applied to a project and its components can be done in a
    //! thread-safe manner.

    class Project
    {
      public:

        Project();
        ~Project();

        int   load(const char* fname);
        int   save(const char* fname);
        void  clear();

        int   initSolver(bool initFlows);
        int   runSolver(int* t);
        int   advanceSolver(int* dt);

        int   openOutput(const char* fname);
        int   saveOutput();

        int   openReport(const char* fname);
        void  writeSummary();
        void  writeResults(int t);
        int   writeReport();

        void  writeMsg(const std::string& msg);
        void  writeMsgLog(std::ostream& out);
        void  writeMsgLog();
        Network* getNetwork() { return &network; }

      private:

        Network        network;        //!< pipe network to be analyzed.
        HydEngine      hydEngine;      //!< hydraulic simulation engine.
        QualEngine     qualEngine;     //!< water quality simulation engine.
        OutputFile     outputFile;     //!< binary output file for saved results.
        std::string    inpFileName;    //!< name of project's input file.
        std::string    outFileName;    //!< name of project's binary output file.
        std::string    tmpFileName;    //!< name of project's temporary binary output file.
        std::string    rptFileName;    //!< name of project's report file.
        std::ofstream  rptFile;        //!< reporting file stream.

        // Project status conditions
        bool           networkEmpty;
        bool           hydEngineOpened;
        bool           qualEngineOpened;
        bool           outputFileOpened;
        bool           solverInitialized;
        bool           runQuality;

        void           finalizeSolver();
        void           closeReport();
    };
}
#endif

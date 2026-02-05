/* EPANET 3.1
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

///////////////////////////////////////////////
// Implementation of EPANET 3.1's API library  //
///////////////'''''///////////////////////////

// TO DO:
// - finish implementing all of the functions declared in EPANET3.H
// - provide a brief comment on what each function does

#include "epanet3.h"
#include "Core/project.h"
#include "Core/datamanager.h"
#include "Core/constants.h"
#include "Core/error.h"
#include "Utilities/utilities.h"

#include <iostream>
#include <iomanip>
#include <time.h>
#include <string>

using namespace Epanet;
using namespace std;

#define project(p) ((Project *)p)

extern "C" {

//-----------------------------------------------------------------------------

int EN_getVersion(int* version)
{
    *version = VERSION;
    return 0;
}

//-----------------------------------------------------------------------------

int EN_runEpanet(const char* inpFile, const char* rptFile, const char* outFile)
{
    std::cout << "\n... EPANET Version 3.0\n";

    // ... declare a Project variable and an error indicator
    Project p;
    int err = 0;

	// Output files in text format, and variables which are existing in output file 

	//std::ofstream myfile("D:\\EPANET_3\\Networks\\RWC\\Onizuka\\Onizuka1986CCV.txt");
	//std::ofstream myfile("D:\\EPANET_3\\Networks\\RWC\\Nault2016\\Nault2016CCV.txt");
	std::ofstream myfile("D:\\EPANET_3\\Networks\\RWC\\Nault2018\\Nault2018.txt");
	//std::ofstream myfile("D:\\EPANET_3\\Networks\\RWC\\Nault2018EPA3-EPS.txt");
	
	int IndexP5, IndexP8, IndexT1, IndexJ2;
	double f5, f8, h1, h2;          //Onizuka (1986)
	
	int IndexJ7, IndexJ18, IndexJ25, IndexP34, IndexPS2P3, IndexP2, IndexP25, IndexR1, IndexR2, IndexR3;
	double h7, h18, h25, f34, fPS2P3, f2, f25; // Nault and Karney (2016)   

	int IndexJ70, IndexJ105, IndexP447, IndexP1364;
	double h70, h105, f447, f1364; // Nault and Karney(2018)

    // ... initialize execution time clock
    clock_t start_t = clock();

	
    for (;;)
    {
        // ... open the command line files and load network data
        if ( (err = p.openReport(rptFile)) ) break;
        std::cout << "\n    Reading input file ...";
        if ( (err = p.load(inpFile)) ) break;
        if ( (err = p.openOutput(outFile)) ) break;
        p.writeSummary();

        // ... initialize the solver
        std::cout << "\n    Initializing solver ...";
        if ( (err = p.initSolver(false)) ) break;
        std::cout << "\n    ";

		/*int ErrorP5 = EN_getLinkIndex("P5", &IndexP5, p.getNetwork());
		int ErrorP8 = EN_getLinkIndex("P8", &IndexP8, p.getNetwork());
		int ErrorT1 = EN_getNodeIndex("T1", &IndexT1, p.getNetwork());
		int ErrorJ2 = EN_getNodeIndex("J2", &IndexJ2, p.getNetwork());

		double ErrorValP5 = EN_getLinkValue(IndexP5, EN_FLOW, &f5, p.getNetwork());
		double ErrorValP8 = EN_getLinkValue(IndexP8, EN_FLOW, &f8, p.getNetwork());
		double ErrorValT1 = EN_getNodeValue(IndexT1, EN_HEAD, &h1, p.getNetwork());
		double ErrorValJ2 = EN_getNodeValue(IndexJ2, EN_HEAD, &h2, p.getNetwork()); //  Onizuka (1986) */

		/*int ErrorJ7 = EN_getNodeIndex("J7", &IndexJ7, p.getNetwork());
		int ErrorJ18 = EN_getNodeIndex("J18", &IndexJ18, p.getNetwork());
		int ErrorJ25 = EN_getNodeIndex("J25", &IndexJ25, p.getNetwork());
		int ErrorP34 = EN_getLinkIndex("P34", &IndexP34, p.getNetwork());
		int ErrorPS2P3 = EN_getLinkIndex("PS2-P3", &IndexPS2P3, p.getNetwork());
		int ErrorP2 = EN_getLinkIndex("P2", &IndexP2, p.getNetwork());
		int ErrorP25 = EN_getLinkIndex("P25", &IndexP25, p.getNetwork());

		double ErrorValJ7 = EN_getNodeValue(IndexJ7, EN_HEAD, &h7, p.getNetwork());
		double ErrorValJ18 = EN_getNodeValue(IndexJ18, EN_HEAD, &h18, p.getNetwork());
		double ErrorValJ25 = EN_getNodeValue(IndexJ25, EN_HEAD, &h25, p.getNetwork());
		double ErrorValP34 = EN_getLinkValue(IndexP34, EN_FLOW, &f34, p.getNetwork());
		double ErrorValPS2P3 = EN_getLinkValue(IndexPS2P3, EN_FLOW, &fPS2P3, p.getNetwork());
		double ErrorValP2 = EN_getLinkValue(IndexP2, EN_FLOW, &f2, p.getNetwork());
		double ErrorValP25 = EN_getLinkValue(IndexP25, EN_FLOW, &f25, p.getNetwork());  // Nault and Karney (2016) */

		int ErrorJ70 = EN_getNodeIndex("J70", &IndexJ70, p.getNetwork());
		int ErrorJ105 = EN_getNodeIndex("J105", &IndexJ105, p.getNetwork());
		int ErrorP447 = EN_getLinkIndex("P447", &IndexP447, p.getNetwork());
		int ErrorP1364 = EN_getLinkIndex("P1364", &IndexP1364, p.getNetwork());
		
		double ErrorValJ70 = EN_getNodeValue(IndexJ70, EN_HEAD, &h70, p.getNetwork());
		double ErrorValJ105 = EN_getNodeValue(IndexJ105, EN_HEAD, &h105, p.getNetwork());
		double ErrorValP447 = EN_getLinkValue(IndexP447, EN_FLOW, &f447, p.getNetwork());  
		double ErrorValP1364 = EN_getLinkValue(IndexP1364, EN_FLOW, &f1364, p.getNetwork());  //  Nault and Karney (2018) */

        // ... step through each time period
        int t = 0;
        int tstep = 0;
        do
        {
            std::cout << "\r    Solving network at "                     //r
                << Utilities::getTime(t+tstep) << " hrs ...        ";
			
            // ... run solver to compute hydraulics
            err = p.runSolver(&t);
            p.writeMsgLog();

            // ... advance solver to next period in time while solving for water quality
            if ( !err ) err = p.advanceSolver(&tstep);

			/*double ErrorValP5 = EN_getLinkValue(IndexP5, EN_FLOW, &f5, p.getNetwork());
			double ErrorValP8 = EN_getLinkValue(IndexP8, EN_FLOW, &f8, p.getNetwork());
			double ErrorValT1 = EN_getNodeValue(IndexT1, EN_HEAD, &h1, p.getNetwork());
			double ErrorValJ2 = EN_getNodeValue(IndexJ2, EN_HEAD, &h2, p.getNetwork());

			myfile << t << " " << f5 << " " << f8 << " " << h1 << " " << h2 << "\n"; // */

			/*double ErrorValJ7 = EN_getNodeValue(IndexJ7, EN_HEAD, &h7, p.getNetwork());
			double ErrorValJ18 = EN_getNodeValue(IndexJ18, EN_HEAD, &h18, p.getNetwork());
			double ErrorValJ25 = EN_getNodeValue(IndexJ25, EN_HEAD, &h25, p.getNetwork());
			double ErrorValP34 = EN_getLinkValue(IndexP34, EN_FLOW, &f34, p.getNetwork());
			double ErrorValPS2P3 = EN_getLinkValue(IndexPS2P3, EN_FLOW, &fPS2P3, p.getNetwork());
			double ErrorValP2 = EN_getLinkValue(IndexP2, EN_FLOW, &f2, p.getNetwork());
			double ErrorValP25 = EN_getLinkValue(IndexP25, EN_FLOW, &f25, p.getNetwork());
			myfile << t << " " << h7 << " " << h18 << " " << h25 << " " << f34 << " " << fPS2P3 << " " << f2 << " " << f25 << "\n"; // */

			double ErrorValJ70 = EN_getNodeValue(IndexJ70, EN_HEAD, &h70, p.getNetwork());
			double ErrorValJ105 = EN_getNodeValue(IndexJ105, EN_HEAD, &h105, p.getNetwork());
			double ErrorValP447 = EN_getLinkValue(IndexP447, EN_FLOW, &f447, p.getNetwork());
			double ErrorValP1364 = EN_getLinkValue(IndexP1364, EN_FLOW, &f1364, p.getNetwork());

			myfile << t << " " << h70 << " " << h105 << " " << f447 << " " << f1364 << "\n"; // */

        } while (tstep > 0 && !err );
        break;
    }

    // ... simulation was successful
    if ( !err )
    {
        // ... report execution time
        clock_t end_t = clock();
        double cpu_t = ((double) (end_t - start_t)) / CLOCKS_PER_SEC;
        std::stringstream ss;
        ss << "\n  Simulation completed in ";
        p.writeMsg(ss.str());
        ss.str("");
        if ( cpu_t < 0.001 ) ss << "< 0.001 sec.";
        else ss << std::setprecision(3) << cpu_t << " sec.";
        p.writeMsg(ss.str());

        // ... report simulation results
        std::cout << "\n    Writing report ...                           ";
        err = p.writeReport();
        std::cout << "\n    Simulation completed.                         \n";
        std::cout << "\n... EPANET completed in " << ss.str() << "\n";
    }

    if ( err )
    {
        p.writeMsgLog();
        std::cout << "\n\n    There were errors. See report file for details.\n";
        return err;
    }
    return 0;
}

//-----------------------------------------------------------------------------

EN_Project EN_createProject()
{
    Project* p = new Project();
    return (EN_Project *)p;
}

//-----------------------------------------------------------------------------

int EN_deleteProject(EN_Project p)
{
    delete (Project *)p;
    return 0;
}

//-----------------------------------------------------------------------------

int EN_loadProject(const char* fname, EN_Project p)
{
    return project(p)->load(fname);
}

//-----------------------------------------------------------------------------

int EN_saveProject(const char* fname, EN_Project p)
{
    return project(p)->save(fname);
}

//-----------------------------------------------------------------------------

int EN_clearProject(EN_Project p)
{
    project(p)->clear();
    return 0;
}

//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////
//  NOT SURE IF THIS METHOD WORKS CORRECTLY -- NEEDS TESTING  //
////////////////////////////////////////////////////////////////
int EN_cloneProject(EN_Project pClone, EN_Project pSource)
{
    if ( pSource == nullptr || pClone == nullptr ) return 102;
    int err = 0;
    std::string tmpFile;
    if ( Utilities::getTmpFileName(tmpFile) )
    {
        try
        {
            EN_saveProject(tmpFile.c_str(), pSource);
            EN_loadProject(tmpFile.c_str(), pClone);
        }
        catch (ENerror const& e)
        {
	        project(pSource)->writeMsg(e.msg);
            err = e.code;
  	    }
        catch (...)
        {
            err = 208; //Unspecified error
        }
        if ( err > 0 )
        {
            EN_clearProject(pClone);
        }
        remove(tmpFile.c_str());
        return err;
    }
    return 208;
}

//-----------------------------------------------------------------------------

int EN_runProject(EN_Project p)    // <<=============  TO BE COMPLETED
{
    return 0;
}

//-----------------------------------------------------------------------------

int EN_initSolver(int initFlows, EN_Project p)
{
    return project(p)->initSolver(initFlows);
}

//-----------------------------------------------------------------------------

int EN_runSolver(int* t, EN_Project p)
{
    return project(p)->runSolver(t);
}

//-----------------------------------------------------------------------------

int EN_advanceSolver(int *dt, EN_Project p)
{
    return project(p)->advanceSolver(dt);
}

//-----------------------------------------------------------------------------

int EN_openOutputFile(const char* fname, EN_Project p)
{
    return project(p)->openOutput(fname);
}

//-----------------------------------------------------------------------------

int EN_saveOutput(EN_Project p)
{
    return project(p)->saveOutput();
}

//-----------------------------------------------------------------------------

int EN_openReportFile(const char* fname, EN_Project p)
{
    return project(p)->openReport(fname);
}

//-----------------------------------------------------------------------------

int EN_writeReport(EN_Project p)
{
    return project(p)->writeReport();
}

//-----------------------------------------------------------------------------

int EN_writeSummary(EN_Project p)
{
    project(p)->writeSummary();
    return 0;
}

//-----------------------------------------------------------------------------

int EN_writeResults(int t, EN_Project p)
{
    project(p)->writeResults(t);
    return 0;
}

//-----------------------------------------------------------------------------

int EN_writeMsgLog(EN_Project p)
{
    project(p)->writeMsgLog();
    return 0;
}

//-----------------------------------------------------------------------------

int EN_getCount(int element, int* result, EN_Project p)
{
    return DataManager::getCount(element, result, project(p)->getNetwork());
}

//-----------------------------------------------------------------------------

int EN_getNodeIndex(char* name, int* index, EN_Project p)
{
    return DataManager::getNodeIndex(name, index, project(p)->getNetwork());
}

//-----------------------------------------------------------------------------

int EN_getNodeId(int index, char* id, EN_Project p)
{
    return DataManager::getNodeId(index, id, project(p)->getNetwork());
}

//-----------------------------------------------------------------------------

int EN_getNodeType(int index, int* type, EN_Project p)
{
    return DataManager::getNodeType(index, type, project(p)->getNetwork());
}

//-----------------------------------------------------------------------------

int EN_getNodeValue(int index, int param, double* value, EN_Project p)
{
    return DataManager::getNodeValue(index, param, value, project(p)->getNetwork());
}

//-----------------------------------------------------------------------------

int EN_getLinkIndex(char* name, int* index, EN_Project p)
{
    return DataManager::getLinkIndex(name, index, project(p)->getNetwork());
}

//-----------------------------------------------------------------------------

int EN_getLinkId(int index, char* id, EN_Project p)
{
    return DataManager::getLinkId(index, id, project(p)->getNetwork());
}

//-----------------------------------------------------------------------------

int EN_getLinkType(int index, int* type, EN_Project p)
{
    return DataManager::getLinkType(index, type, project(p)->getNetwork());
}

//-----------------------------------------------------------------------------

int EN_getLinkNodes(int index, int* fromNode, int* toNode, EN_Project p)
{
    return DataManager::getLinkNodes(index, fromNode, toNode,
                                     project(p)->getNetwork());
}

//-----------------------------------------------------------------------------

int EN_getLinkValue(int index, int param, double* value, EN_Project p)
{
   return DataManager::getLinkValue(index, param, value, project(p)->getNetwork());
}

int EN_setLinkValue(int index, int param, double value, EN_Project p)
{
	return DataManager::setLinkValue(index, param, value, project(p)->setNetwork());
}


}  // end of namespace

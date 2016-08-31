/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

 //////////////////////////////////////////////
 //  Implementation of the HydEngine class.  //
 //////////////////////////////////////////////

 // TO DO:
 // - add support for Rule-Based controls
 // - add support for saving hydraulics results to a binary file

#include "hydengine.h"
#include "network.h"
#include "error.h"
#include "Solvers/hydsolver.h"
#include "Solvers/matrixsolver.h"
#include "Elements/link.h"
#include "Elements/tank.h"
#include "Elements/pattern.h"
#include "Elements/control.h"
#include "Utilities/utilities.h"

#include <iostream>
#include <algorithm>
#include <iomanip>
#include <string>
#include <vector>
using namespace std;

//static const string s_Balancing  = " Balancing the network:";
static const string s_Unbalanced =
    "  WARNING - network is unbalanced. Flows and pressures may not be correct.";
static const string s_UnbalancedHalted =
    "  Network is unbalanced. Simulation halted by user.";
static const string s_IllConditioned =
    "  Network is numerically ill-conditioned. Simulation halted.";
static const string s_Balanced   = "  Network balanced in ";
static const string s_Trials     = " trials.";
static const string s_Deficient  = " nodes were pressure deficient.";
static const string s_ReSolve1   =
    "\n    Re-solving network with these made fixed grade.";
static const string s_Reductions1 =  " nodes require demand reductions.";
static const string s_ReSolve2    =
    "\n    Re-solving network with these reductions made.";
static const string s_Reductions2 =
    " nodes require further demand reductions to 0.";

//-----------------------------------------------------------------------------

//  Constructor

HydEngine::HydEngine() :
    engineState(HydEngine::CLOSED),
    network(nullptr),
    hydSolver(nullptr),
    matrixSolver(nullptr),
    saveToFile(false),
    halted(false),
    startTime(0),
    rptTime(0),
    hydStep(0),
    currentTime(0),
    timeOfDay(0),
    peakKwatts(0.0)
{
}

//-----------------------------------------------------------------------------

//  Destructor

HydEngine::~HydEngine()
{
    close();

//    cout << "\nHydEngine destructed.\n";
}

//-----------------------------------------------------------------------------

//  Opens the hydraulic engine.

void HydEngine::open(Network* nw)
{
    // ... close a currently opened engine

    if (engineState != HydEngine::CLOSED) close();
    network = nw;

    // ... create hydraulic sub-models (can throw exception)

    network->createHeadLossModel();
    network->createDemandModel();
    network->createLeakageModel();

    // ... create and initialize a matrix solver

    matrixSolver = MatrixSolver::factory(
        network->option(Options::MATRIX_SOLVER), network->msgLog);
    if ( matrixSolver == nullptr )
    {
        throw SystemError(SystemError::MATRIX_SOLVER_NOT_OPENED);
    }
    initMatrixSolver();

    // ... create a hydraulic solver

    hydSolver = HydSolver::factory(
        network->option(Options::HYD_SOLVER), network, matrixSolver);
    if ( hydSolver == nullptr )
    {
        throw SystemError(SystemError::HYDRAULIC_SOLVER_NOT_OPENED);
    }
    engineState = HydEngine::OPENED;
}

//-----------------------------------------------------------------------------

//  Initializes the hydraulic engine.

void HydEngine::init(bool initFlows)
{
    if (engineState == HydEngine::CLOSED) return;

    for (Link* link : network->links)
    {
        link->initialize(initFlows);        // flows & status
        link->setResistance(network);       // pipe head loss resistance
    }

    for (Node* node : network->nodes)
    {
        node->initialize(network);          // head, quality, volume, etc.
    }

    int patternStep = network->option(Options::PATTERN_STEP);
    int patternStart = network->option(Options::PATTERN_START);
    for (Pattern* pattern : network->patterns)
    {
        pattern->init(patternStep, patternStart);
    }

    halted = 0;
    currentTime = 0;
    hydStep = 0;
    startTime = network->option(Options::START_TIME);
    rptTime = network->option(Options::REPORT_START);
    peakKwatts = 0.0;
    engineState = HydEngine::INITIALIZED;
    timeStepReason = "";
}

//-----------------------------------------------------------------------------

//  Solves network hydraulics at the current point in time.

int  HydEngine::solve(int* t)
{
    if ( engineState != HydEngine::INITIALIZED ) return 0;
    if ( network->option(Options::REPORT_STATUS) )
    {
        network->msgLog << endl << "  Hour " <<
            Utilities::getTime(currentTime) << timeStepReason;
    }

    *t = currentTime;
    timeOfDay = (currentTime + startTime) % 86400;
    updateCurrentConditions();

    //if ( network->option(Options::REPORT_TRIALS) )  network->msgLog << endl;
    int trials = 0;
    int statusCode = hydSolver->solve(hydStep, trials);

    if ( statusCode == HydSolver::SUCCESSFUL && isPressureDeficient() )
    {
        statusCode = resolvePressureDeficiency(trials);
    }
    reportDiagnostics(statusCode, trials);
    if ( halted ) throw SystemError(SystemError::HYDRAULICS_SOLVER_FAILURE);
    return statusCode;
}

//-----------------------------------------------------------------------------

//  Advances the simulation to the next point in time.

void HydEngine::advance(int* tstep)
{
    *tstep = 0;
    if ( engineState != HydEngine::INITIALIZED ) return;

    // ... save current results to hydraulics file
    //if ( saveToFile ) errcode = hydWriter.writeResults(hydState, hydTime);

    // ... if time remains, find time (hydStep) until next hydraulic event

    hydStep = 0;
    int timeLeft = network->option(Options::TOTAL_DURATION) - currentTime;
    if ( halted ) timeLeft = 0;
    if ( timeLeft > 0  )
    {
        hydStep = getTimeStep();
        if ( hydStep > timeLeft ) hydStep = timeLeft;
    }
    *tstep = hydStep;

    // ... update energy usage and tank levels over the time step

    updateEnergyUsage();
    updateTanks();

    // ... advance time counters

    currentTime += hydStep;
    if ( currentTime >= rptTime )
    {
        rptTime += network->option(Options::REPORT_STEP);
    }

    // ... advance time patterns

    updatePatterns();
}

//-----------------------------------------------------------------------------

//  Closes the hydraulic solver.

void HydEngine::close()
{
    if ( engineState == HydEngine::CLOSED ) return;
    delete matrixSolver;
    matrixSolver = nullptr;
    delete hydSolver;
    hydSolver = nullptr;
    engineState = HydEngine::CLOSED;

    //... Other objects created in HydEngine::open() belong to the
    //    network object and are deleted by it.
}

//-----------------------------------------------------------------------------

//  Initializes the matrix equation solver.

void HydEngine::initMatrixSolver()
{
    int nodeCount = network->count(Element::NODE);
    int linkCount = network->count(Element::LINK);
    try
    {
        // ... place the start/end node indexes of each network link in arrays

        vector<int> node1(linkCount);
        vector<int> node2(linkCount);
        for (int k = 0; k < linkCount; k++)
        {
            node1[k] = network->link(k)->fromNode->index;
            node2[k] = network->link(k)->toNode->index;
        }

        // ...  initialize the matrix solver

        matrixSolver->init(nodeCount, linkCount, (int *)&node1[0], (int *)&node2[0]);
    }
    catch (...)
    {
        throw;
    }
}

//-----------------------------------------------------------------------------

//  Updates network conditions at start of current time step.

void HydEngine::updateCurrentConditions()
{
    // ... identify global demand multiplier and pattern factor

    double multiplier = network->option(Options::DEMAND_MULTIPLIER);
    double patternFactor = 1.0;

////  Need to change from a pattern index to a patttern pointer  /////
////  or to update DEMAND_PATTERN option if current pattern is deleted.  ////

    int    p = network->option(Options::DEMAND_PATTERN);
    if ( p >= 0 ) patternFactor = network->pattern(p)->currentFactor();

    // ... update node conditions

    for (Node* node : network->nodes)
    {
        // ... find node's full target demand for current time period
        node->findFullDemand(multiplier, patternFactor);

        // ... set its fixed grade state (for tanks & reservoirs)
        node->setFixedGrade();
    }

    // ... update link conditions

    for (Link* link : network->links)
    {
        // ... open a temporarily closed link
        //if ( link->status >= Link::TEMP_CLOSED ) link->status = Link::LINK_OPEN;

        // ... apply pattern-based pump or valve setting
        link->applyControlPattern(network->msgLog);
    }

    // ... apply simple conditional controls

    for (Control* control : network->controls)
    {
        control->apply(network, currentTime, timeOfDay);
    }
}

//-----------------------------------------------------------------------------

bool HydEngine::isPressureDeficient()
{
    int count = 0;
    for (Node* node : network->nodes)
    {
        // ... This only gets evaluated for the CONSTRAINED demand model
        if ( node->isPressureDeficient(network) ) count++;
    }
    if ( count > 0 && network->option(Options::REPORT_TRIALS) )
    {
        network->msgLog << "\n\n    " << count << s_Deficient;
    }
    return (count > 0);
}

//-----------------------------------------------------------------------------

int HydEngine::resolvePressureDeficiency(int& trials)
{
    int trials2 = 0;
    int trials3 = 0;
    int trials4 = 0;
    int count1 = 0;
    int count2 = 0;
    bool reportTrials = ( network->option(Options::REPORT_TRIALS) );

    // ... re-solve network hydraulics with the pressure deficient junctions
    //     set to fixed grade (which occurred in isPressureDeficient())

    if ( reportTrials ) network->msgLog << s_ReSolve1;
    int statusCode = hydSolver->solve(hydStep, trials2);
    if ( statusCode == HydSolver::FAILED_ILL_CONDITIONED ) return statusCode;

    // ... adjust actual demands for the pressure deficient junctions

    for (Node* node : network->nodes)
    {
        if ( node->type() == Node::JUNCTION && node->fixedGrade )
        {
            node->actualDemand = min(node->actualDemand, node->fullDemand);
            node->actualDemand = max(0.0, node->actualDemand);
            if ( node->actualDemand < node->fullDemand ) count1++;
            node->fixedGrade = false;
        }
    }

    // ... re-solve once more with the reduced demands at the affected junctions

    if (reportTrials )
    {
        network->msgLog << "\n\n    " << count1 << s_Reductions1;
        network->msgLog << s_ReSolve2;
    }
    statusCode = hydSolver->solve(hydStep, trials3);

    // ... check once more for any remaining pressure deficiencies

    for (Node* node : network->nodes)
    {
        if ( node->isPressureDeficient(network) )
        {
            count2++;

            // ... remove fixed grade status set in isPressureDeficient
            //     and make actual demand 0
            node->fixedGrade = false;
            node->actualDemand = 0.0;
        }
    }

     // ... if there are any, then re-solve once more

    if ( count2 > 0 )
    {
        if ( reportTrials )
        {
            network->msgLog << "\n    " << count2 << s_Reductions2;
            network->msgLog << s_ReSolve2 << "\n";
        }
        statusCode = hydSolver->solve(hydStep, trials4);
    }

    trials += trials2 + trials3 + trials4;
    return statusCode;
}

//-----------------------------------------------------------------------------

//  Report diagnostics on current hydraulics run.

void HydEngine::reportDiagnostics(int statusCode, int trials)
{
    if ( statusCode == HydSolver::FAILED_ILL_CONDITIONED ||
       ( statusCode == HydSolver::FAILED_NO_CONVERGENCE  &&
         network->option(Options::IF_UNBALANCED) == Options::STOP ))
        halted = true;

    if ( network->option(Options::REPORT_TRIALS) ) network->msgLog << endl;
    if ( network->option(Options::REPORT_STATUS) )
    {
        network->msgLog << endl;
        switch (statusCode)
        {
        case HydSolver::SUCCESSFUL:
            network->msgLog <<	s_Balanced << trials << s_Trials;
            break;
        case HydSolver::FAILED_NO_CONVERGENCE:
            if ( halted ) network->msgLog << s_UnbalancedHalted;
            else          network->msgLog << s_Unbalanced;
            break;
        case HydSolver::FAILED_ILL_CONDITIONED:
            network->msgLog << s_IllConditioned;
            break;
        }
        network->msgLog << endl;
    }
}

//-----------------------------------------------------------------------------

//  Determines the next time step to advance hydraulics.

int HydEngine::getTimeStep()
{
    // ... normal time step is user-supplied hydraulic time step

    string reason ;
    int tstep = network->option(Options::HYD_STEP);
    int n = currentTime / tstep + 1;
    tstep = n * tstep - currentTime;
    timeStepReason = "";

    // ... adjust for time until next reporting period

    int t = rptTime - currentTime;
    if ( t > 0 && t < tstep )
    {
        tstep = t;
        timeStepReason = "";
    }

    // ... adjust for time until next time pattern change

    tstep = timeToPatternChange(tstep);

    // ... adjust for shortest time to fill or drain a tank

    tstep = timeToCloseTank(tstep);

    // ... adjust for shortest time to activate a simple control

    tstep = timeToActivateControl(tstep);
    return tstep;
}

//-----------------------------------------------------------------------------

//  Finds shortest time until next change for all time patterns.

int HydEngine::timeToPatternChange(int tstep)
{
    Pattern* changedPattern = nullptr;
    for (Pattern* pattern : network->patterns)
    {
        int t = pattern->nextTime(currentTime) - currentTime;
        if ( t > 0 && t < tstep )
        {
            tstep = t;
            changedPattern = pattern;
        }
    }
    if ( changedPattern )
    {
        timeStepReason = "  (change in Pattern " + changedPattern->name + ")";
    }
    return tstep;
}

//-----------------------------------------------------------------------------

//  Finds the shortest time to completely fill or empty all tanks.

int HydEngine::timeToCloseTank(int tstep)
{
    Tank* closedTank = nullptr;
    for (Node* node : network->nodes)
    {
        // ... check if node is a tank

        if ( node->type() == Node::TANK )
        {
            // ... find the time to fill (or empty) the tank

            Tank* tank = static_cast<Tank*>(node);
            int t = tank->timeToVolume(tank->minVolume);
            if ( t <= 0 ) t = tank->timeToVolume(tank->maxVolume);

            // ... compare this time with current time step

            if ( t > 0 && t < tstep )
            {
                tstep = t;
                closedTank = tank;
            }
        }
    }
    if ( closedTank )
    {
        timeStepReason = "  (Tank " + closedTank->name + " closed)";
    }
    return tstep;
}

//-----------------------------------------------------------------------------

//  Finds the shortest time to activate a simple control.

int HydEngine::timeToActivateControl(int tstep)
{
    bool activated = false;
    for (Control* control : network->controls)
    {
        int t = control->timeToActivate(network, currentTime, timeOfDay);
        if ( t > 0 && t < tstep )
        {
            tstep = t;
            activated = true;
        }
    }
    if ( activated ) timeStepReason = "  (control activated)";
    return tstep;
}

//-----------------------------------------------------------------------------

//  Updates energy usage over the current time step.

void HydEngine::updateEnergyUsage()
{
    // ... use a nominal time step of 1 day if running a single period analysis

    int dt = hydStep;
    if ( network->option(Options::TOTAL_DURATION) == 0 ) dt = 86400;
    if ( dt == 0 ) return;

    // ... update energy usage for each pump link over the time step

    double totalKwatts = 0.0;
    for (Link* link : network->links)
    {
        totalKwatts += link->updateEnergyUsage(network, dt);
    }

    // ... update peak energy usage over entire simulation

    peakKwatts = max(peakKwatts, totalKwatts);
}

//-----------------------------------------------------------------------------

//  Updates tank area and volume over the current time step.

void HydEngine::updateTanks()
{
    for (Node* node : network->nodes)
    {
    	if ( node->type() == Node::TANK )
        {
            Tank* tank = static_cast<Tank*>(node);
            tank->pastHead = tank->head;
            tank->pastVolume = tank->volume;
            tank->pastOutflow = tank->outflow;
            node->fixedGrade = true;
            tank->updateVolume(hydStep);
            tank->updateArea();
        }
    }
}

//-----------------------------------------------------------------------------

//  Advances all time patterns.

void HydEngine::updatePatterns()
{
    for (Pattern* pattern : network->patterns)
    {
        pattern->advance(currentTime);
    }
}

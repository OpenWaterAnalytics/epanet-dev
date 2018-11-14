### Introduction

This document summarizes how EPANET 3 differs from EPANET 2.

_ALL ITEMS ARE SUBJECT TO CHANGE AS EPANET 3 UNDERGOES ADDITIONAL DEVELOPMENT._

### Input File Format

EPANET 3 is capable of reading EPANET 2 input files. However several keywords in the **[OPTIONS]** section of the file were changed to provide more clarity and consistency. The changes are as follows:

| Old Keyword        | New Keyword          |
| ------------------ | -------------------- |
| UNITS              | FLOW_UNITS           |
| PRESSURE           | PRESSURE_UNITS       |
| HEADLOSS           | HEADLOSS_MODEL       |
| QUALITY            | QUALITY_MODEL        |
| VISCOSITY          | SPECIFIC_VISCOSITY   |
| DIFFUSIVITY        | SPECIFIC_DIFFUSIVITY |
| SPECIFIC GRAVITY   | SPECIFIC_GRAVITY     |
| TRIALS             | MAXIMUM_TRIALS       |
| ACCURACY           | RELATIVE_ACCURACY    |
| UNBALANCED         | IF_UNBALANCED        |
| PATTERN            | DEMAND_PATTERN       |
| DEMAND MULTIPLIER  | DEMAND_MULTIPLIER    |
| EMITTER EXPONENT   | EMITTER_EXPONENT     |
| TOLERANCE          | QUALITY_TOLERANCE    |
| HYDRAULICS         | HYDRAULICS_FILE      |
| MAP                | MAP_FILE             |
| CHECKFREQ          | deprecated           |
| MAXCHECK           | deprecated           |
| DAMPlIMIT          | deprecated           |

In addition, a number of new keywords have been added to the **[OPTIONS]** section to implement new features added to the code. These are listed below and are covered in more detail in other sections of this document.

| New Option Keyword   | Meaning                                          |
| -------------------- | ------------------------------------------------ |
| DEMAND_MODEL         | Choice of pressure-dependent demand model        |
| LEAKAGE_MODEL        | Choice of pipe leakage model                     |
| HYDRAULIC_SOLVER     | Choice of hydraulic solver                       |
| MATRIX_SOLVER        | Choice of linear equation solver                 |
| HEAD_TOLERANCE       | Tolerance in satisfying head loss equations      |
| FLOW_TOLERANCE       | Tolerance in satisfying flow continuity          |
| FLOW_CHANGE_LIMIT    | Convergence limit on link flow change            |
| STEP_SIZING          | Choice of step sizing method in solving hydraulics |
| TIME_WEIGHT          | Backwards difference weight for dynamic tanks    |
| MINIMUM_PRESSURE     | Global pressure below which demand is zero       |
| SERVICE_PRESSURE     | Global pressure above which full demand is met   |
| PRESSURE_EXPONENT    | Global exponent in power demand model            |
| LEAKAGE_COEFF1       | Global coefficient used for pipe leakage         |
| LEAKAGE_COEFF2       | Global coefficient used for pipe leakage         |
| QUALITY_NAME         | Name of chemical in water quality analysis       |
| QUALITY_UNITS        | Concentration units of water quality chemical    |
| TRACE_NODE           | Name of source node in a water quality trace     |

The **_DURATION_** keyword in the **[TIMES]** section of the file has been replaced with **_TOTAL DURATION_** to make it compatible with the other [TIMES] options that use a pair of keywords.

In the **[REPORT]** section of the file, **_PAGESIZE_** has been deprecated and the **_STATUS_** option has been split into two separate options, **_STATUS_** **YES/NO** for status reporting and **_TRIALS_** **YES/NO** for reporting individual trials of the hydraulic solver.

Finally, the names used to identify network elements (e.g., nodes, links, patterns, curves, etc.) are no longer limited to 31 characters. They are, however, still case sensitive.

### Models and Solvers

The computational elements in EPANET can be broken down into models for representing particular aspects of network behavior (e.g., pipe head loss, pressure-dependent demands, water quality reactions, etc.) and solvers that compute output results (e.g., hydraulic, sparse matrix, and water quality solvers). EPANET 3 is structured so that its models and solvers are represented by a set of abstract classes that adhere to a specific interface. This makes it easier (in theory) to add alternative models and solvers in the future with a minimum of disruption to the existing code base. The table below lists the **[OPTIONS]** keywords used to specify a choice of model or solver. 

| Option Keyword   | Available Choices              |
| ---------------- | ------------------------------ |
| HEADLOSS_MODEL   | H-W (Hazen-Williams)           |
|                  | D-W (Darcy-Weisbach)           |
|                  | C-M (Chezy-Manning)            |
| DEMAND_MODEL     | FIXED                          |
|                  | CONSTRAINED                    |
|                  | POWER                          |
|                  | LOGISTIC                       |
| LEAKAGE_MODEL    | NONE                           |
|                  | POWER                          |
|                  | FAVAD                          |
| QUALITY_MODEL    | NONE                           |
|                  | CHEMICAL                       |
|                  | TRACE                          |
|                  | AGE                            |
| HYDRAULIC_SOLVER | GGA (Global Gradient Algorithm |
| QUALITY_SOLVER   | LTD (Lagrangian Time Driven)   |
| MATRIX_SOLVER    | SPARSPAK                       |

Right now there is only a single choice of each solver but additional alternatives could be added at a later date. Implementations of the various models and solvers can be found in the _Models/_ and _Solvers/_ directories, respectively. 

### API (Toolkit) Usage

The way in which the API functions are used to analyze a network have changed. The differences between the version 2 and 3 APIs can be summarized as follows:
* Function names now begin with an "EN_" prefix followed by a name in lower camel case.
* You must first call **_EN_createProject_** to create an EPANET project object before using other API functions that include the project as an argument. This allows one to use parallel processing on a number of different projects in a thread safe manner.
* **_ENopen_** has been replaced with **_EN_openReport_**, **_EN_openOutput_**, and **_EN_loadProject_**.
* **_EN_initSolver_** replaces **_ENopenH_**, **_ENinitH_**, **_ENopenQ_** and **_ENinitQ_**.
* **_EN_runSolver_** replaces **_ENrunH_** for computing hydraulics at the current time period.
* **_EN_advanceSolver_** replaces **_ENnextH_**, **_EN_runQ_**, and **_ENnextQ_**. It advances the simulation to the next time when hydraulics are to be updated while computing water quality over this time interval as need be.
* As implied by the previous item, water quality is now run simultaneously with hydraulics. There is no need to run hydraulics for all time periods first before solving for water quality.
* There is no longer a need for functions like **_ENcloseH_** and **_ENcloseQ_**. You only need to call **_EN_deleteProject_** after all analysis of a project has been completed to insure that all memory is properly released.

Here is an example of using the new API to run a complete simulation:

```
#include "epanet3.h"
void myEpanet3Runner(char* inpFile, char* rptFile)
{
    int t = 0, dt = 0;
    EN_Project p = EN_createProject();
    EN_openReportFile(rptFile, p);
    EN_loadProject(inpFile, p);
    EN_openOutputFile("", p);
    EN_initSolver(EN_NOINITFLOW, p);
    do {
        EN_runSolver(&t, p);
        EN_advanceSolver(&dt, p);
    while ( dt > 0 );
    EN_writeReport(p);
    EN_deleteProject(p);
}
```

### Pressure Dependent Demands

EPANET 3 offers four different ways of handling consumer demands at network nodes through its **_DEMAND_MODEL_** option:
- **FIXED**         (demands are fixed values not dependent on pressure)
- **CONSTRAINED**   (demands are reduced so that no net negative pressures occur)
- **POWER**         (a node's demand varies as a power function of pressure)
- **LOGISTIC**      (a node's demand varies as a logistic function of pressure).

The **_MINIMUM_PRESSURE_** option is used with choices 2 - 4 to set a pressure below which demand will be 0. Its default value is 0. The **_SERVICE_PRESSURE_** option is used with choices 3 and 4 to set a pressure above which a node's full demand is supplied. The **_PRESSURE_EXPONENT_** option sets the exponent used for **POWER** function demands.

The implementation of these methods can be found in the _Models/demandmodel.h_ and _Models/demandmodel.cpp_ files as well as in _Core/hydengine.cpp_ for the **CONSTRAINED** option.

### Pipe Leakage

Pressure dependent pipe leakage can now be modeled using the new **_LEAKAGE_MODEL_** option. The choices are:
- **NONE** for no leakage modeling.
- **POWER** : <a href="https://www.codecogs.com/eqnedit.php?latex=Leak&space;=&space;C_1P^{C_2}" target="_blank"><img src="https://latex.codecogs.com/gif.latex?Leak&space;=&space;C_1P^{C_2}" title="Leak = C_1P^{C_2}" /></a>
- **FAVAD** : <a href="https://www.codecogs.com/eqnedit.php?latex=Leak&space;=&space;0.6&space;\sqrt{2g}&space;\left&space;(&space;C_1&space;P^{0.5}&space;&plus;&space;C_2&space;P^{1.5}&space;\right)" target="_blank"><img src="https://latex.codecogs.com/gif.latex?Leak&space;=&space;0.6&space;\sqrt{2g}&space;\left&space;(&space;C_1&space;P^{0.5}&space;&plus;&space;C_2&space;P^{1.5}&space;\right)" title="Leak = 0.6 \sqrt{2g} \left ( C_1 P^{0.5} + C_2 P^{1.5} \right)" /></a>

For the **Power** model the leakage rate is in flow units/1000 length units of pipe (ft or m), P is the average pressure (psi or m) across the pipe and C1 and C2 are user supplied coefficients. For the **FAVAD** model, the leakage rate is cfs/1000 ft (or cms/km) of pipe, P is the average pressure head (ft or m) across the pipe, C1 is the area (sq. ft. or sq. m) of leaks per 1000 ft or m of pipe and C2 is the change in leakage area per change in pressure head per 1000 length units of pipe.

The leakage coefficients can be supplied on a global basis using the new option keywords **_LEAKAGE_COEFF1_** and **_LEAKAGE_COEFF2_**. Their default values are 0. The coefficients can also be be supplied on an individual pipe basis by adding a **[LEAKAGE]** section to the input file where each line contains a pipe name and a pair of coefficients. Computed leakage rates are split 50-50 to outflow from the pipe's end nodes.

Pipe leakage is implemented in the files _Models/leakagemodel.h_, _Models/leakagemodel.cpp_, and _Core/hydbalance.cpp_.

### Hydraulic Convergence Criteria

In EPANET 2, convergence to an acceptable hydraulic solution occurred when the sum of all link flow changes divided by the sum of all link flows was below the **_ACCURACY_** (re-named to **_RELATIVE_ACCURACY_**) option value. Unfortunately meeting this criterion did not always guarantee that the network was hydraulically balanced (e.g., that the head loss computed from the flow in each link equaled the difference between the computed heads at the link's end nodes). EPANET 3 introduces a more rigorous set of criteria consisting of the following options:
- **_HEAD_TOLERANCE_**
-- the difference between the computed head loss in each link and the heads at its end nodes must be below this value.
- **_FLOW_TOLERANCE_**
-- the difference between inflow and outflow at all non-fixed grade nodes must be below this value.
- **_FLOW_CHANGE_LIMIT_**
-- the largest change in link flow rate must be below this value.

The units of the head tolerance are feet (or meters) while the user's choice of flow units apply to the other two. A value of 0 indicates that the criterion doesn't apply. See _Core/hydbalance.h_ and _Core/hydbalance.cpp_ for how the convergence criteria are calculated.

### Tank Dynamics

EPANET 2 used a foward difference (or Euler) method to approximate the change in storage tank water level over a time step as a function of the current flows within the network. This could cause instabilities to occur in and around tanks that were hydraulically coupled to one another. To remedy this, EPANET 3 models tank dynamics using the time weighted implicit formula proposed by Todini (2011). A new option named **_TIME_WEIGHT_** sets the weight to be used in this formulation. A value of 0 maintains the forward difference formula of EPANET 2 while a value of 1.0 results in a fully backwards difference formulation.

### Low Resistance Pipes

When using the Hazen-Williams (H-W) head loss equation, EPANET 2's hydraulic solver can have problems converging for networks with low resistance or zero flow pipes. To help avoid this, EPANET 3 employs a linear head loss relationship whenever the H-W head loss gradient drops below a minimum threshold (currently set at 1.0e-6 ft/cfs). 

### Check Valves

The method used by EPANET 2 to find the head loss through an active check valve produced discontinuous gradients and was overly complicated. EPANET 3 uses a simple continuous "barrier" function that is added onto the pipe's normal head loss. It rises rapidly when flow is in the wrong direction but otherwise approaches 0. The details of this function can be found in the _Models/headlossmodel.cpp_ file.

### Output Variables

The following quantities have been added to the set of computed results that can be retrieved through the API toolkit:
* node actual demand (which can be less than the full demand due to pressure dependency)
* node total outflow (consisting of actual demand, leakage flow, and emitter flow)
* link leakage flow
* link water quality

### Binary Output File Format

The binary file used to store computed results has been modified from the EPANET 2 format to include the new variables reported by EPANET 3. In addition, the file no longer includes the ID names and design data of nodes and links in its prolog section. The new binary file format can be gleaned from the code found in _Output/outputfile.cpp_.

### Additional Changes
* Emitters are prevented from having flow back into the network.
* The gradient of the Darcy-Weisbach head loss equation now includes the derivative of the friction factor.
* Proper adjustment of the efficiency curve is now made for variable speed pumps.

### References
Todini (2011) Extending the global gradient algorithm to unsteadyﬂow extended period simulations of water distributionsystems. Journal of Hydroinformatics, 13(2):167-180

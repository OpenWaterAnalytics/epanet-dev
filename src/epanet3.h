/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#ifndef EPANET3_H_
#define EPANET3_H_

//************************************
//  EPANET 3's API FUNCTION LIBRARY  *
//************************************

typedef void * EN_Project;

enum NodeParams {

    EN_ELEVATION,    //0
    EN_BASEDEMAND,   //1
    EN_BASEPATTERN,  //2
    EN_EMITTERFLOW,  //3
    EN_INITQUAL,     //4
    EN_SOURCEQUAL,   //5
    EN_SOURCEPAT,    //6
    EN_SOURCETYPE,   //7

    EN_TANKLEVEL,    //8
    EN_FULLDEMAND,   //9
    EN_HEAD,         //10
    EN_PRESSURE,     //11
    EN_QUALITY,      //12
    EN_SOURCEMASS,   //13

    EN_INITVOLUME,   //14
    EN_MIXMODEL,     //15
    EN_MIXZONEVOL,   //16
    EN_TANKDIAM,     //17
    EN_MINVOLUME,    //18
    EN_VOLCURVE,     //19
    EN_MINLEVEL,     //20
    EN_MAXLEVEL,     //21
    EN_MIXFRACTION,  //22
    EN_TANK_KBULK,   //23
    EN_TANKVOLUME,   //24

    EN_ACTUALDEMAND, //25
    EN_OUTFLOW};     //26

enum LinkParams {
    EN_DIAMETER,     //0
    EN_LENGTH,       //1
    EN_ROUGHNESS,    //2
    EN_MINORLOSS,    //3
    EN_INITSTATUS,   //4
    EN_INITSETTING,  //5
    EN_KBULK,        //6
    EN_KWALL,        //7
    EN_FLOW,         //8
    EN_VELOCITY,     //9
    EN_HEADLOSS,     //10
    EN_STATUS,       //11
    EN_SETTING,      //12
    EN_ENERGY,       //13
    EN_LINKQUAL,     //14
    EN_LEAKCOEFF1,   //15
    EN_LEAKCOEFF2,   //16
    EN_LEAKAGE};     //17

enum TimeParams {
    EN_DURATION,     //0
    EN_HYDSTEP,      //1
    EN_QUALSTEP,     //2
    EN_PATTERNSTEP,  //3
    EN_PATTERNSTART, //4
    EN_REPORTSTEP,   //5
    EN_REPORTSTART,  //6
    EN_RULESTEP,     //7
    EN_STATISTIC,    //8
    EN_PERIODS,      //9
    EN_STARTDATE};   //10

enum ElementCounts {
    EN_NODECOUNT,    //0
    EN_TANKCOUNT,    //1
    EN_LINKCOUNT,    //2
    EN_PATCOUNT,     //3
    EN_CURVECOUNT,   //4
    EN_CONTROLCOUNT, //5
    EN_RULECOUNT,    //6
    EN_RESVCOUNT};   //7

enum NodeTypes {
    EN_JUNCTION,     //0
    EN_RESERVOIR,    //1
    EN_TANK};        //2

enum LinkTypes {
    EN_CVPIPE,       //0
    EN_PIPE,         //1
    EN_PUMP,         //2
    EN_PRV,          //3
    EN_PSV,          //4
    EN_PBV,          //5
    EN_FCV,          //6
    EN_TCV,          //7
    EN_GPV};         //8

enum QualModelTypes {
    EN_NONE,         //0
    EN_CHEM,         //1
    EN_AGE ,         //2
    EN_TRACE};       //3

enum QualSourceTypes {
    EN_CONCEN,       //0
    EN_MASS,         //1
    EN_SETPOINT,     //2
    EN_FLOWPACED};   //3

enum FlowUnitsTypes {
    EN_CFS,          //0
    EN_GPM,          //1
    EN_MGD,          //2
    EN_IMGD,         //3
    EN_AFD,          //4
    EN_LPS,          //5
    EN_LPM,          //6
    EN_MLD,          //7
    EN_CMH,          //8
    EN_CMD};         //9

enum OptionTypes {
    EN_TRIALS,       //0
    EN_ACCURACY,     //1
    EN_QUALTOL,      //2
    EN_EMITEXPON,    //3
    EN_DEMANDMULT,   //4
    EN_HYDTOL,       //5
    EN_MINPRESSURE,  //6
    EN_MAXPRESSURE,  //7
    EN_PRESSEXPON,   //8
    EN_NETLEAKCOEFF1,  //9
    EN_NETLEAKCOEFF2}; //10

enum ControlTypes {
    EN_LOWLEVEL,     //0
    EN_HILEVEL,      //1
    EN_TIMER,        //2
    EN_TIMEOFDAY};   //3

enum StatisticTypes {
    EN_AVERAGE,      //1
    EN_MINIMUM,      //2
    EN_MAXIMUM,      //3
    EN_RANGE};       //4

enum TankMixTypes {
    EN_MIX1,         //0
    EN_MIX2,         //1
    EN_FIFO,         //2
    EN_LIFO};        //3

enum InitFlowTypes {
    EN_NOINITFLOW,   //0
    EN_INITFLOW};    //1


#ifdef __cplusplus
extern "C" {
#endif

int        EN_getVersion(int *);
int        EN_runEpanet(const char* inpFile, const char* rptFile, const char* outFile);

EN_Project EN_createProject();
int        EN_cloneProject(EN_Project pClone, EN_Project pSource);
int        EN_deleteProject(EN_Project p);

int        EN_loadProject(const char* fname, EN_Project p);
int        EN_runProject(EN_Project p);
int        EN_saveProject(const char* fname, EN_Project p);
int        EN_clearProject(EN_Project p);

int        EN_initSolver(int initFlows, EN_Project p);
int        EN_runSolver(int* t, EN_Project p);
int        EN_advanceSolver(int* dt, EN_Project p);

int        EN_openOutputFile(const char* fname, EN_Project p);
int        EN_saveOutput(EN_Project p);

int        EN_openReportFile(const char* fname, EN_Project p);
int        EN_writeReport(EN_Project p);
int        EN_writeSummary(EN_Project p);
int        EN_writeResults(int t, EN_Project p);
int        EN_writeMsgLog(EN_Project p);

int        EN_getCount(int, int *, EN_Project);
int        EN_getNodeIndex(char *, int *, EN_Project);
int        EN_getNodeId(int, char *, EN_Project);
int        EN_getNodeType(int, int *, EN_Project);
int        EN_getNodeValue(int, int, double *, EN_Project);

int        EN_getLinkIndex(char *, int *, EN_Project);
int        EN_getLinkId(int, char *, EN_Project);
int        EN_getLinkType(int, int *, EN_Project);
int        EN_getLinkNodes(int, int *, int *, EN_Project);
int        EN_getLinkValue(int, int, double *, EN_Project);

//==================================================================================
/*        TO BE ADDED

int       EN_getOption(int, double *, EN_Project);
int       EN_getTimeParam(int, long *, EN_Project);
int       EN_getFlowUnits(int *, EN_Project);
int       EN_getPatternIndex(char *, int *, EN_Project);
int       EN_getPatternId(int, char *, EN_Project);
int       EN_getPatternPeriod(int, int*, EN_Project);
int       EN_getPatternLen(int, int *,EN_Project);
int       EN_getPatternValue(int, int, double *, EN_Project);
int       EN_getQualModel(int *, EN_Project);
int       EN_getError(int, char *, int, EN_Project);
int       EN_getControl(int, int *, int *, double *, int *, double *, EN_Project);

int       EN_setControl(int, int, int, double, int, double, EN_Project);
int       EN_setNodeValue(int, int, double, EN_Project);
int       EN_setLinkValue(int, int, double, EN_Project);
int       EN_addPattern(char *, EN_Project);
int       EN_setPattern(int, double *, int, EN_Project);
int       EN_setPatternValue(int, int, double, EN_Project);
int       EN_setTimeParam(int, int, EN_Project);
int       EN_setOption(int, double, EN_Project);
int       EN_setStatusReport(int, EN_Project);
int       EN_setQualType(int, char *, char *, char *, EN_Project);

int       EN_createNode(char *, int, EN_Project);
int       EN_createLink(char *, int, int, int, EN_Project);
int       EN_createCurve(char *, int, int, double *, double *, EN_Project);
int       EN_createFixedPattern(char *, int, int, double *, EN_Project);
int       EN_createVarPattern(char *, int, int *, double *, EN_Project);
int       EN_createControl(char *, EN_Project);

int       EN_deleteNode(char *, EN_Project);
int       EN_deleteLink(char *, EN_Project);
int       EN_deleteCurve(char *, EN_Project);
int       EN_deletePattern(char *, EN_Project);
*/

#ifdef __cplusplus
}
#endif

#endif /* EPANET3_H_ */

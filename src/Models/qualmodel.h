/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file qualmodel.h
//! \brief Describes the QualModel class.

#ifndef QUALMODEL_H_
#define QUALMODEL_H_

#include <string>

class Network;
class Pipe;
class Tank;
class Node;

//! \class QualModel
//! \brief The interface for a water quality analysis model.
//!
//! QualModel is an abstract class from which a concrete water
//! quality analysis model is derived. Three such models are
//! currently available - Chemical, Trace, and Age.

class QualModel
{
  public:

    enum QualModelType {
        NOQUAL,          // no quality model
        AGE,             // water age
        TRACE,           // source tracing
        CHEM};           // chemical constituent

    QualModel(int _type);
    virtual ~QualModel() = 0;

    static  QualModel* factory(const std::string model);

    virtual bool isReactive()
	{ return false; }

    virtual void init(Network* nw)
	{ }

    virtual void findMassTransCoeff(Pipe* pipe)
	{ }

    virtual double pipeReact(Pipe* pipe, double c, double tstep)
	{ return c; }

    virtual double tankReact(Tank* tank, double c, double tstep)
	{ return c; }

    virtual double findTracerAdded(Node* node, double qIn)
    { return 0.0; }

    int type;
};


//-----------------------------------------------------------------------------
//! \class ChemModel
//! \brief Reactive chemical model.
//-----------------------------------------------------------------------------

class ChemModel : public QualModel
{
  public:
    ChemModel();
    bool   isReactive() { return reactive; }
    void   init(Network* nw);
    void   findMassTransCoeff(Pipe* pipe);
    double pipeReact(Pipe* pipe, double c, double tstep);
    double tankReact(Tank* tank, double c, double tstep);

  private:
    bool    reactive;         // true if chemical is reactive
    double  diffus;           // chemical's diffusuivity (ft2/sec)
    double  viscos;           // water kin. viscosity (ft2/sec)
    double  Sc;               // Schmidt number
    double  pipeOrder;        // pipe bulk fluid reaction order
    double  tankOrder;        // tank bulk fluid reaction order
    double  wallOrder;        // pipe wall reaction order
    double  massTransCoeff;   // a pipe's mass transfer coeff. (ft/sec)
    double  pipeUcf;          // volume conversion factor for pipes
    double  tankUcf;          // volume conversion factor for tanks
    double  cLimit;           // min/max concentration limit (mass/ft3)

    bool    setReactive(Network* nw);
    double  findBulkRate(double kb, double order, double c);
    double  findWallRate(double kw, double d, double order, double c);
};


//-----------------------------------------------------------------------------
//! \class TraceModel
//! \brief Source tracing model.
//-----------------------------------------------------------------------------

class TraceModel : public QualModel
{
  public:
    TraceModel() : QualModel(TRACE) { }
    void   init(Network* nw);
    double findTracerAdded(Node* node, double qIn);

  private:
    Node* traceNode;
};


//-----------------------------------------------------------------------------
//! \class AgeModel
//! \brief Water age model.
//-----------------------------------------------------------------------------

class AgeModel : public QualModel
{
  public:
    AgeModel() : QualModel(AGE) { }
    bool   isReactive() { return true; }
    double pipeReact(Pipe* pipe, double age, double tstep);
    double tankReact(Tank* tank, double age, double tstep);
};

#endif

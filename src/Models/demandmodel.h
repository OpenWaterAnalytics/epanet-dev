/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file demandmodel.h
//! \brief Describes the DemandModel class and its sub-classes.

#ifndef DEMANDMODEL_H_
#define DEMANDMODEL_H_

#include <string>

class Junction;

//! \class DemandModel
//! \brief The interface for a pressure-dependent demand model.
//!
//! DemandModel is an abstract class from which a concrete demand
//! model is derived. Four such models are currently available -
//! Fixed, Constrained, Power, and Logistic.

class DemandModel
{
  public:
    DemandModel();
    DemandModel(double expon_);
    virtual ~DemandModel() = 0;
    static  DemandModel* factory(const std::string model, double expon_);

    /// Finds demand flow and its derivative as a function of head.
    virtual double findDemand(Junction* junc, double h, double& dqdh);

    /// Changes fixed grade status depending on pressure deficit.
    virtual bool isPressureDeficient(Junction* junc) { return false; }

  protected:
    double expon;
};


//-----------------------------------------------------------------------------
//! \class  FixedDemandModel
//! \brief A demand model where demands are fixed independent of pressure.
//-----------------------------------------------------------------------------

class FixedDemandModel : public DemandModel
{
  public:
    FixedDemandModel();
};


//-----------------------------------------------------------------------------
//! \class  ConstrainedDemandModel
//! \brief A demand model where demands are reduced based on available pressure.
//-----------------------------------------------------------------------------

class ConstrainedDemandModel : public DemandModel
{
  public:
    ConstrainedDemandModel();
    bool isPressureDeficient(Junction* junc);
    double findDemand(Junction* junc, double p, double& dqdh);
};


//-----------------------------------------------------------------------------
//! \class  PowerDemandModel
//! \brief A demand model where demand varies as a power function of pressure.
//-----------------------------------------------------------------------------

class PowerDemandModel : public DemandModel
{
  public:
    PowerDemandModel(double expon_);
    double findDemand(Junction* junc, double p, double& dqdh);
};


//-----------------------------------------------------------------------------
//! \class  LogisticDemandModel
//! \brief A demand model where demand is a logistic function of pressure.
//-----------------------------------------------------------------------------

class LogisticDemandModel : public DemandModel
{
  public:
    LogisticDemandModel(double expon_);
    double findDemand(Junction* junc, double p, double& dqdh);

  private:
    double a, b;  // logistic function coefficients
    void  setCoeffs(double pMin, double pFull);
};

#endif

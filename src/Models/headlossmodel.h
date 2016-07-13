/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file headlossmodel.h
//! \brief Describes the HeadLossModel class and its sub-classes.

#ifndef HEADLOSSMODEL_H_
#define HEADLOSSMODEL_H_

#include <string>

class Pipe;

//! \class HeadLossModel
//! \brief The interface for a pipe head loss model.
//!
//! HeadLossModel is an abstract class from which a concrete head
//! loss computational model is derived. Three such models are
//! currently available - Hazen-Williams, Darcy-Weisbach and
//! Chezy-Manning.

class HeadLossModel
{
  public:

    /// Constructor/destructor
    HeadLossModel(double viscos);
    virtual ~HeadLossModel() = 0;

    /// Factory method for creating a headloss model
    static HeadLossModel* factory(const std::string model, double viscos);

    /// Static methods for closed links & links with check valves
    static void findClosedHeadLoss(double flow, double& headLoss, double& gradient);
    static void addCVHeadLoss(double flow, double& headLoss, double& gradient);

    /// Methods that set model parameters
    void    setViscosity(double v) { viscosity = v;}
    virtual void setResistance(Pipe* pipe) = 0;

    /// Method that finds a link's head loss and its gradient
    virtual void findHeadLoss(
                     Pipe* pipe, double flow, double& headLoss, double& gradient) = 0;

  protected:
    double  viscosity;         //!< water viscosity (ft2/sec)
};


//-----------------------------------------------------------------------------
//! \class HW_HeadLossModel
//! \brief The Hazen-Williams head loss model.
//-----------------------------------------------------------------------------

class HW_HeadLossModel : public HeadLossModel
{
  public:
    HW_HeadLossModel(double viscos);
    void   setResistance(Pipe* pipe);
    void   findHeadLoss(Pipe* pipe, double flow, double& headLoss, double& gradient);
};


//-----------------------------------------------------------------------------
//! \class DW_HeadLossModel
//! \brief The Darcy-Weisbach head loss model.
//-----------------------------------------------------------------------------

class DW_HeadLossModel : public HeadLossModel
{
  public:
    DW_HeadLossModel(double viscos);
    void   setResistance(Pipe* pipe);
    void   findHeadLoss(Pipe* pipe, double flow, double& headLoss, double& gradient);
};


//-----------------------------------------------------------------------------
//! \class CM_HeadLossModel
//! \brief The Chezy-Manning head loss model.
//-----------------------------------------------------------------------------

class CM_HeadLossModel : public HeadLossModel
{
  public:
    CM_HeadLossModel(double viscos);
    void   setResistance(Pipe* pipe);
    void   findHeadLoss(Pipe* pipe, double flow, double& headLoss, double& gradient);
};

#endif

/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

//! \file leakagemodel.h
//! \brief Describes the LeakageModel class and its sub-classes.

#ifndef LEAKAGEMODEL_H_
#define LEAKAGEMODEL_H_

#include <string>

//! \class LeakageModel
//! \brief The interface for a pipe leakage model.
//!
//! LeakageModel is an abstract class from which a concrete leakage
//! model is derived. Two such models are currently available -
//! a power function model and a fixed-and-variable-areas discharge
//! (FAVAD) model.

class LeakageModel
{
  public:
    LeakageModel();
    virtual ~LeakageModel();
    static  LeakageModel* factory(
                                  const std::string model,
                                  const double ucfLength_,
                                  const double ucfFlow_);
    virtual double findFlow(double c1,
                            double c2,
                            double length,
                            double h,
                            double& dqdh) = 0;

  protected:
    double lengthUcf;
    double flowUcf;
    double pressureUcf;
};

//-----------------------------------------------------------------------------
//! \class PowerLeakageModel
//! \brief Pipe leakage rate varies as a power function of pipe pressure.
//-----------------------------------------------------------------------------

class PowerLeakageModel : public LeakageModel
{
  public:
    PowerLeakageModel(const double ucfLength_, const double ucfFlow_);
    double findFlow(double flowCoeff, double expon, double length, double h,
                    double& dqdh);
};

//-----------------------------------------------------------------------------
//! \class FavadLeakageModel
//! \brief Pipe leakage is computed using an orifice equation where the
//!        leak area is a function of pipe pressure.
//-----------------------------------------------------------------------------

class FavadLeakageModel : public LeakageModel
{
  public:
    FavadLeakageModel(const double ucfLength_);
    double findFlow(double area, double slope, double length, double h, double& dqdh);
};

#endif /* LEAKAGEMODEL_H_ */

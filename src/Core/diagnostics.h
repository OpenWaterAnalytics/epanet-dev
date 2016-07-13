/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file diagnostics.h
//! \brief A class that performs diagnostics on EPANET input and output.

#ifndef DIAGNOSTICS_H_
#define DIAGNOSTICS_H_

class Network;

struct Diagnostics
{
    void  validateNetwork(Network* nw);
};

#endif // DIAGNOSTICS_H_

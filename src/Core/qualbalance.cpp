/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

 ////////////////////////////////////////////////
 //  Implementation of the QualBalance class.  //
 ////////////////////////////////////////////////

 // TO DO:
 // - include units with the quantities listed in the Mass Balance table

#include "qualbalance.h"
#include "Core/network.h"
#include "Elements/node.h"

using namespace std;

//-----------------------------------------------------------------------------

void QualBalance::init(const double initMassStored)
{
    initMass = initMassStored;
    inflowMass = 0.0;
    outflowMass = 0.0;
    reactedMass = 0.0;
    storedMass = initMass;
}

//-----------------------------------------------------------------------------

void QualBalance::writeBalance(ostream& msgLog)
{
    msgLog <<   "\n  Water Quality Mass Balance"
           <<   "\n  --------------------------";
    msgLog <<   "\n  Initial Storage           " << initMass / 1.e6;
    msgLog <<   "\n  Mass Inflow               " << inflowMass / 1.e6;
    msgLog <<   "\n  Mass Outflow              " << outflowMass / 1.e6;
    msgLog <<   "\n  Mass Reacted              " << reactedMass / 1.e6;
    msgLog <<   "\n  Final Storage             " << storedMass / 1.e6;

    double massIn = initMass + inflowMass;
    double massOut = outflowMass + reactedMass + storedMass;
    double pctDiff = (massIn - massOut);
    if ( massIn > 0.0 ) pctDiff = 100.0 * pctDiff / massIn;
    else if (massOut > 0.0 ) pctDiff = 100.0 * pctDiff / massOut;
    else pctDiff = 0.0;
    msgLog <<   "\n  Percent Imbalance         " << pctDiff << "\n";
}

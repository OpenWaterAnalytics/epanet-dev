/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

//! \file main.cpp
//! \brief The main function used to run EPANET from the command line.

#include "epanet3.h"

#include <iostream>

int main(int argc, char* argv[])
{
    //... check number of command line arguments
    if (argc < 3)
    {
        std::cout << "\nCorrect syntax is: epanet3 inpFile rptFile (outFile)\n";
        return 0;
    }

    //... retrieve file names from command line
    const char* f1 = argv[1];
    const char* f2 = argv[2];
    const char* f3 = "";
    if (argc > 3) f3 = argv[3];

    // ... run a full EPANET analysis
    EN_runEpanet(f1, f2, f3);
    //system("PAUSE");
    return 0;
}

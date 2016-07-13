/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file inputreader.h
//! \brief Describes the InputReader class.

#ifndef INPUTREADER_H_
#define INPUTREADER_H_

#include <iostream>
#include <sstream>

class Network;
class InputParser;

//! \class InputReader
//! \brief Reads lines of project input data from a text file.
//!
//! The reader makes two passes through the project's input file - once using the
//! ObjectParser to identify and create each element (node, link, pattern, etc.)
//! in the network and then using the PropertyParser to read the properties
//! assigned to each of these elements. This two-pass approach allows the
//! description of the elements to appear in any order in the file.

class InputReader
{
  public:

    enum Section {
        TITLE,              JUNCTION,           RESERVOIR,          TANK,
        PIPE,               PUMP,               VALVE,              PATTERN,
        CURVE,              CONTROL,            RULE,               EMITTER,
        DEMAND,             STATUS,             ROUGHNESS,          LEAKAGE,
        ENERGY,             QUALITY,            SOURCE,             REACTION,
        MIXING,             OPTION,             TIME,               REPORT,
        COORD,              VERTICES,           LABEL,              MAP,
        BACKDROP,           TAG,                END
    };

    InputReader();
    ~InputReader() {}

    void readFile(const char* inpFile, Network* network);

  protected:

    std::istringstream sin;         //!< string stream containing a line of input
    int                errcount;    //!< error count
    int                section;     //!< file section being processed

    void parseFile(std::istream& fin, InputParser& parser);
    void trimLine(std::string& line);
    void findSection(std::string& token);
};

#endif

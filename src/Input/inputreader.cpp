/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

#include "inputreader.h"
#include "inputparser.h"
#include "Core/network.h"
#include "Core/error.h"
#include "Utilities/utilities.h"

#include <fstream>
using namespace std;

//-----------------------------------------------------------------------------

static const int    MAXERRS = 10;             // maximum number of input errors allowed
static const string WHITESPACE = " \t\n\r";   // whitespace characters

//-----------------------------------------------------------------------------

// Names of input file sections

const char* sections[]  =
{
    "[TITLE",           "[JUNCTION",        "[RESERVOIR",       "[TANK",
    "[PIPE",            "[PUMP",            "[VALVE",           "[PATTERN",
    "[CURVE",           "[CONTROL",         "[RULE",            "[EMITTER",
    "[DEMAND",          "[STATUS",          "[ROUGHNESS",       "[LEAKAGE",
    "[ENERGY",          "[QUALITY",         "[SOURCE",          "[REACTION",
    "[MIXING",          "[OPTION",          "[TIME",            "[REPORT",
    "[COORD",           "[VERTICES",        "[LABEL",           "[MAP",
    "[BACKDROP",        "[TAG",             "[END",             0
};

//-----------------------------------------------------------------------------

//  InputReader constructor

InputReader::InputReader(): errcount(0), section(-1)
{
}

//-----------------------------------------------------------------------------

//  Read the contents of an EPANET input file.
//  Makes two passes through the file, one that identifies all
//  objects contained in the file, and another that extracts the
//  properties of these objects.

void InputReader::readFile(const char* inpFile, Network* network)
{
    // ... initialize current input section

    section = -1;

    // ... open the input file

    ifstream fin(inpFile, ios::in);
    if (!fin.is_open()) throw FileError(FileError::CANNOT_OPEN_INPUT_FILE);
    try
    {
        // ... parse object names from the file

        ObjectParser objectParser(network);
        parseFile(fin, objectParser);

        // ... parse object properties from the file

        PropertyParser propertyParser(network);
        parseFile(fin, propertyParser);
        fin.close();
    }

    // ... catch and re-throw any exception thrown by the parsing process

    catch (...)
    {
        fin.close();
        throw;
    }
}

//-----------------------------------------------------------------------------

//  Read and parse each line of the input file.

void InputReader::parseFile(istream& fin, InputParser& parser)
{
    string line;
    string token;

    // ... reset input file

    fin.clear();
    fin.seekg(0);
    section = -1;

    // ... read each line from input file

    for (;;)
    {
        if ( errcount >= MAXERRS ) break;
        getline(fin, line, '\n');
        if (fin.fail())
        {
            if ( fin.eof() ) break;    // end of file reached - normal termination
        }

        // ... remove any comment from input line

        trimLine(line);

        // ... read 1st token from input line

        sin.clear();                   // clear string stream status flags
        sin.str(line);                 // assign input line to string stream
        sin >> token;                  // read 1st token from string stream

        // ... skip blank lines

        if ( sin.fail() || token[0] == '\0' ) continue;
        try
        {
            // ... see if at start of new input section

            if ( token[0] == '[' ) findSection(token);

            // ... otherwise parse input line of data

            else parser.parseLine(line, section);
        }
        catch (InputError& e)
        {
            errcount++;
            if ( section >= 0 )
            {
                parser.network->msgLog << e.msg << " at following line of " <<
                    sections[section] << "] section:\n";
            }
            else
            {
                parser.network->msgLog << e.msg << " at following line of file:\n";
            }
            parser.network->msgLog << line << "\n";
        }
        catch (...)
        {
            errcount++;
        }
    }

    // ... throw general input file exception if errors were found

    if ( errcount > 0 ) throw InputError(InputError::ERRORS_IN_INPUT_DATA, "");
}

//-----------------------------------------------------------------------------

//  Trim a comment from a line of text.

void InputReader::trimLine(string& line)
{
    // ... skip any characters following a ';'

    size_t pos = line.find(";");
    if (pos != string::npos) line.erase(pos);

    // ... trim any trailing whitespace

    pos = line.find_last_not_of(WHITESPACE);
    if (pos != string::npos) line.erase(pos + 1);
}

//-----------------------------------------------------------------------------

//  Find which input section keyword a string token matches.

void InputReader::findSection(string& token)
{
    int newSection = Utilities::findMatch(token, sections);
    if (newSection < 0) throw InputError(InputError::INVALID_KEYWORD, token);
    section = newSection;
}

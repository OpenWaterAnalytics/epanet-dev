/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "utilities.h"

#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace std;

static const string s_Day     = "DAY";
static const string s_Hour    = "HOUR";
static const string  s_Minute = "MIN";
static const string  s_Second = "SEC";
static const string  s_AM     = "AM";
static const string  s_PM     = "PM";

//-----------------------------------------------------------------------------
//  Gets the name of a temporary file
//-----------------------------------------------------------------------------

bool Utilities::getTmpFileName(string& fname)
{
#ifdef _WIN32
    TCHAR tmpFileName[MAX_PATH];
    TCHAR tmpPathName[MAX_PATH];
    int rtnValue = GetTempPath(MAX_PATH, tmpPathName);
    if ( rtnValue == 0 || rtnValue > MAX_PATH ) return false;
    rtnValue = GetTempFileName(tmpPathName, TEXT("EN"), 0, tmpFileName);
    if ( rtnValue > 0 ) fname = tmpFileName;
#else
    char tmpName[] = "/tmp/epanetXXXXXX";
    int fd = mkstemp(tmpName);
    if ( fd == -1 ) return false;
    fname = tmpName;
#endif
    return true;
}

//-----------------------------------------------------------------------------
//  Extracts a file name from a full path
//-----------------------------------------------------------------------------

string Utilities::getFileName(const std::string s)
{
    char sep = '/';
#ifdef _WIN32
    sep = '\\';
#endif

    size_t i = s.rfind(sep, s.length());
    if ( i != string::npos )
    {
        return s.substr(i+1, s.length()-i);
    }
    return s;
}


//-----------------------------------------------------------------------------
// Splits a string into tokens separated by whitespace
//-----------------------------------------------------------------------------

void Utilities::split(vector<string>& tokens, const string& str)
{
    istringstream iss(str);
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter<vector<string> >(tokens));
}

vector<string> Utilities::split(const string& str)
{
    istringstream iss(str);
    istream_iterator<string> begin(iss), end;
    vector<string> tokens(begin, end);
    return tokens;
}

//-----------------------------------------------------------------------------
//  Converts a string to upper case (for ASCII characters only!)
//-----------------------------------------------------------------------------

string Utilities::upperCase(const string& s)
{
    string s1 = s;
    //transform(s1.begin(), s1.end(), s1.begin(), ::toupper);
    for(int i=0; s1[i]!=0; i++) s1[i] = toupper((int)s1[i]);
    return s1;
}

//-----------------------------------------------------------------------------
//   Matches a string with those in a list.
//   Returns -1 if no match is found.
//-----------------------------------------------------------------------------

int Utilities::findMatch(const string& s, const char* slist[])
{
    int i = 0;
    while (slist[i])
    {
        if ( match(s, slist[i]) ) return i;
        i++;
    }
    return -1;
}

//-----------------------------------------------------------------------------
//  Sees if full string matches any of those in a list.
//  Returns -1 if no match is found.
//-----------------------------------------------------------------------------

int Utilities::findFullMatch(const string& s, const char* slist[])
{
    int i = 0;
    while (slist[i])
    {
        if (s.compare(slist[i]) == 0) return i;
        i++;
    }
    return -1;
}

//-----------------------------------------------------------------------------
//  Sees if string s1 matches the first part of s2 - case insensitive
//-----------------------------------------------------------------------------

bool Utilities::match(const string& s1, const string& s2)
{
    // select 1st element of each string

    string::const_iterator p1 = s1.begin(),
                           p2 = s2.begin();

    // while characters remain in each string,

    while ( p1 != s1.end() && p2 != s2.end() )
    {
        // compare upper-case characters

        if ( toupper((int)*p1) != toupper((int)*p2) ) return false;
        p1++;
        p2++;
    }
    return true;
}

//-----------------------------------------------------------------------------
//  Removes double quotes that surround a string.
//-----------------------------------------------------------------------------

void Utilities::removeQuotes(string& s)
{
    size_t pos = s.find('"');
    if (pos == string::npos) return;
    s.erase(pos, 1);
    pos = s.find('"');
    if (pos != string::npos) s.erase(pos, 1);
}

//-----------------------------------------------------------------------------
//  Convert a time value into seconds.
//-----------------------------------------------------------------------------

int Utilities::getSeconds(const string& strTime, const string& strUnits)
{
    // see if time is in military hr:min:sec format

    if ( strTime.find(':', 0) != string::npos )
    {
        int h = 0, m = 0, s = 0;
        if (sscanf(strTime.c_str(), "%d:%d:%d", &h, &m, &s) == 0) return -1;
        return 3600*h + 60*m + s;
    }

    // retrieve time as a decimal number

    double t;
    if (!parseNumber(strTime, t)) return -1;

    // if no units supplied then convert time in hours to seconds

    if (strUnits.size() == 0) return (int) (3600. * t);

    // determine time units and convert time accordingly

    if (match(strUnits, s_Day) == 0)    return (int) (3600. * 24. * t);
    if (match(strUnits, s_Hour) == 0)   return (int) (3600. * t);
    if (match(strUnits, s_Minute) == 0) return (int) (60. * t);
    if (match(strUnits, s_Second) == 0) return (int) t;

    // if AM/PM supplied, time is in hours and adjust it accordingly

    if (match(strUnits, s_AM))
    {
        if (t >= 13.0) return -1;
        if (t >= 12.0) t -= 12.0;
    }
    else if (match(strUnits, s_PM))
    {
       if (t >= 13.0) return -1;
       if (t < 12.0)  t += 12.0;
    }
    else return -1;

    // convert time from hours to seconds

    return (int) (3600 * t);
}

//-----------------------------------------------------------------------------
//  Converts number of seconds to hrs:min:sec format
//-----------------------------------------------------------------------------

string Utilities::getTime(int seconds)
{
    ostringstream sout;
    int t = seconds;
    int hours = t / 3600;
    t = t - 3600*hours;
    int minutes = t / 60;
    seconds = t - 60*minutes;
    sout.setf(ios::right);
    //sout << setw(3) << setfill(' ') << hours << ":";
    sout << hours << ":";
    sout << setw(2) << setfill('0') << minutes << ":";
    sout << setw(2) << setfill('0') << seconds;
    return sout.str();
}

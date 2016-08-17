/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

//! \file utilities.h
//! \brief Describes the Utilities class.

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <string>
#include <sstream>
#include <vector>

//! \class Utilities
//! \brief Utility functions used throughout EPANET.

class Utilities
{
  public:

/// Template function to purge the contents of a container class.
    template<class Seq> static void purge(Seq& c)
    {
        typename Seq::iterator i;
        for (i = c.begin(); i != c.end(); i++) delete *i;
    	c.clear();
    }

/// Replacement for c++11 to_string (which doesn't work with GNU GCC 4.7.2)
    template < typename T > static std::string to_str( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }

/// Generates the name of a temporary file
    static bool getTmpFileName(std::string& fname);

/// Extracts a file name from a full path
    static std::string getFileName(const std::string s);

/// Converts a string to upper case
    static std::string upperCase(const std::string& s);

/// Matches a string with those in a list (case insensitive).
    static int  findMatch(const std::string& s, const char* slist[]);

/// Matches full string with those in a list.
    static int findFullMatch(const std::string& s, const char* slist[]);

/// Checks if one string is a leading substring of another (case insensitive).
    static bool match(const std::string& s1, const std::string& s2);

/// Converts a string representation of time into a number of seconds.
    static int getSeconds(const std::string& strTime, const std::string& strUnits);

/// Removes double quotes that surround a string
    static void removeQuotes(std::string& s);

/// Converts number of seconds into hr:min:sec string
    static std::string getTime(int seconds);

//! Returns the sign of a number (-1 or +1)
    static int sign(double x)
    { return (x < 0 ? -1 : 1); }

//! Splits a string into tokens separated by whitespace
    static void split(std::vector<std::string>& tokens, const std::string& str);
    static std::vector<std::string> split(const std::string& str);

//! Converts a number to a string
    template <typename T>
    static std::string to_string(T const& value)
    {
    	std::stringstream sstr;
        sstr << value;
        return sstr.str();
    }

//! Converts a numeric string into a floating point value.
    template <typename T>
    static bool parseNumber(const std::string& s, T &x)
    {
        std::stringstream ss(s);
        ss >> x;
        return !ss.fail();
    }

};

#endif

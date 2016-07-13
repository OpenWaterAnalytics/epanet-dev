/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file optionparser.h
//! \brief Description of the OptionParser class.

#ifndef OPTIONPARSER_H_
#define OPTIONPARSER_H_

#include <string>
#include <vector>

class Network;

//! \class OptionParser
//! \brief Parses lines of project option data from a text file.

class OptionParser
{
  public:
    OptionParser();
    void  parseOption(Network* network, std::vector<std::string>& tokens);
    void  parseTimeOption(Network* network, std::vector<std::string>& tokens);
    void  parseEnergyOption(Network* network, std::vector<std::string>& tokens);
    void  parseReactOption(Network* network, std::vector<std::string>& tokens);
    void  parseReportOption(Network* network, std::vector<std::string>& tokens);

  private:
    std::string getEpanet3Keyword(const std::string& s1,
                                  const std::string& s2,
                                        std::string& value);
    void        setOption(const std::string& keyword,
                          const std::string& value,
                          Network* network);
    void        parseQualOption(const std::string& s2,
                                const std::string& s3,
                                Network* network);
    void        parseReportItems(int nodesOrLinks,
                                 Network* network,
                                 int nTokens,
                                 std::string* tokens);
    void        parseReportField(Network* network,
                                 int nTokens,
                                 std::string* tokens);

};

#endif

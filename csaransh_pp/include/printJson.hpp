/*!
 * @file
 * Function to print the results as json
 * */
#ifndef PRINTJSON_CSARANSH_HPP
#define PRINTJSON_CSARANSH_HPP

#include <array>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <helper.hpp>
#include <results.hpp>

namespace csaransh {
void printJson(std::ostream &outfile, const InputInfo &i, const ExtraInfo &ei,
               const resultsT &res);

void resToKeyValue(std::ostream &outfile, const resultsT &res);
void infoToKeyValue(std::ostream &outfile, const InputInfo &i,
                    const ExtraInfo &ei);
void configToKeyValue(std::ostream &outfile, const csaransh::Config &c);
} // namespace csaransh

#endif
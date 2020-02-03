/*!
 * @file
 * functions to read parcas input file and xyz file and use other
 * functions to get the results and print them.
 * */

#ifndef PARCASREADER_CSARANSH_HPP
#define PARCASREADER_CSARANSH_HPP

#include <string>

#include <cluster2features.hpp>
#include <helper.hpp>
#include <results.hpp>

namespace csaransh {

csaransh::ErrorStatus processFile(std::string xyzfile, std::ostream &outfile,
                                  const Config &config, std::string id);

csaransh::resultsT process(csaransh::InputInfo &i, csaransh::ExtraInfo &ei,
                           const Config &config);
} // namespace csaransh
#endif
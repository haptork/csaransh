/*!
 * @file
 * functions to read parcas input file and xyz file and use other
 * functions to get the results and print them.
 * */

#ifndef PARCASREADER_CSARANSH_HPP
#define PARCASREADER_CSARANSH_HPP

#include <string>

#include <xyzReader.hpp>
#include <cluster2features.hpp>
#include <helper.hpp>
#include <results.hpp>

namespace csaransh {

std::pair<csaransh::xyzFileStatus, csaransh::ErrorStatus> processTimeFile(csaransh::InputInfo &info,
                                     csaransh::ExtraInfo &extraInfo,
                                     const csaransh::Config &config, std::istream &infile, csaransh::frameStatus &fs, std::ostream &outfile, bool isFirst);

std::pair<csaransh::ErrorStatus,int> processFileTimeCmd(std::string xyzfileName,
                                            std::ostream &outfile,
                                            const Config &config, int id, const csaransh::InputInfo&, const csaransh::ExtraInfo&, bool);
 
}
#endif
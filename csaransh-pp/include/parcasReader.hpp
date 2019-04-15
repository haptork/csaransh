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

namespace csaransh {
// trim a string from both ends
std::string removeComments(std::string s);

std::pair<csaransh::Info, bool> extractInfoParcas(std::string fname);

std::pair<csaransh::Info, bool> extractInfoLammps(std::string fname);

std::array<std::string, 2> separateDirAndFile(std::string path);

std::pair<std::string, std::string> getInfileFromXyzfile(std::string xyzfile);

csaransh::readStatus processParcasFile(std::string xyzfile, std::ofstream &outfile, int id);

}
#endif
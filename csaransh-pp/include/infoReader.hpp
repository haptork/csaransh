/*!
 * @file
 * functions to read parcas input file and xyz file and use other
 * functions to get the results and print them.
 * */

#ifndef INFOREADER_CSARANSH_HPP
#define INFOREADER_CSARANSH_HPP

#include <string>

#include <cluster2features.hpp>
#include <helper.hpp>

namespace csaransh {

std::pair<std::string, std::string> getInfileFromXyzfile(std::string xyzfile);

std::pair<csaransh::XyzFileType, bool> getSimulationCode(std::string fname);

std::tuple<csaransh::InputInfo, csaransh::ExtraInfo, bool>
extractInfoParcas(std::string fname, std::string ftag);

std::tuple<csaransh::InputInfo, csaransh::ExtraInfo, bool>
extractInfoLammps(std::string fname, std::string ftag);

std::tuple<csaransh::InputInfo, csaransh::ExtraInfo, bool> infoFromStdIn();

} // namespace csaransh
#endif
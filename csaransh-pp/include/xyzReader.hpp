/*!
 * @file
 * functions to read parcas input file and xyz file and use other
 * functions to get the results and print them.
 * */

#ifndef XYZREADER_CSARANSH_HPP
#define XYZREADER_CSARANSH_HPP

#include <map>
#include <string>

#include <helper.hpp>

namespace csaransh {

constexpr auto maxColumnsTry = 50;

enum class frameStatus : bool { prelude, inFrame };

enum class lineStatus : int { garbage, coords, inFrameCoords, frameBorder };

std::pair<csaransh::lineStatus, csaransh::Coords>
getCoord(const std::string &line, const csaransh::frameStatus &fs,
         const csaransh::InputInfo &info, const csaransh::ExtraInfo &ei);

std::pair<csaransh::lineStatus, csaransh::Coords>
getCoordLammps(const std::string &line, const csaransh::frameStatus &fs, int);

std::pair<csaransh::lineStatus, csaransh::Coords>
getCoordParcas(const std::string &line, const csaransh::frameStatus &fs, int);

std::pair<csaransh::lineStatus, std::array<csaransh::Coords, 2>>
getCoordDisplaced(const std::string &line);

} // namespace csaransh
#endif
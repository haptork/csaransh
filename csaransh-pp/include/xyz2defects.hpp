/*!
 * @file
 * implements an O(log(N)) algorithm for detecting defects given a
 * single frame file having xyz coordinates of all the atoms
 * */
#ifndef XYZ2DEFECTS_CSARANSH_HPP
#define XYZ2DEFECTS_CSARANSH_HPP

#include <string>

#include <helper.hpp>
#include <results.hpp>

namespace csaransh {

enum class frameStatus : bool {
  prelude,
  inFrame
};

enum class lineStatus : int {
  garbage,
  coords,
  frameBorder
};

//std::pair<lineStatus, Coords> getCoordParcas(std::string& line);

DefectVecT xyz2defects (const std::string &fname, const Info &info);

}

#endif //XYZ2DEFECTS_CSARANSH_HPP
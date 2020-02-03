
/*!
 * @file
 * Functions to correct the origin to minimize the overall offset of all the
 * atoms
 * */

#ifndef OPTIMIZEORIGIN_CSARANSH_HPP
#define OPTIMIZEORIGIN_CSARANSH_HPP

#include <helper.hpp>

namespace csaransh {

Coords correctOrigin(const std::vector<csaransh::Coords> &atoms,
                     csaransh::Coords origin, double latConst);

Coords estimateOrigin(const std::vector<csaransh::Coords> &atoms,
                      const double &latConst);

} // namespace csaransh
#endif

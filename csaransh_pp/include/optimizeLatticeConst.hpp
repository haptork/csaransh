/*!
 * @file
 * functions that finds lattice constant value that gives minimum offset
 * */

#ifndef OPTIMIZELATCONST_CSARANSH_HPP
#define OPTIMIZELATCONST_CSARANSH_HPP

#include <vector>

#include <helper.hpp>

namespace csaransh {

constexpr auto atomsToIgnore = 1000LL;

double optimizeForOffset(std::vector<csaransh::Coords> &atoms,
                         double minLatConst, double maxLatConst, double step);

double optimizeLatConst(std::vector<csaransh::Coords> &atoms, double latConst);

} // namespace csaransh
#endif

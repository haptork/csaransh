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

// Gives next expected lattice site given a lattice site
// in this way it enumerates all the lattice sites
// Currently only works for bcc
// TODO: extend for fcc

Coords getInitialMax(const Coords &origin, const Coords &maxes);

std::pair<csaransh::DefectVecT, std::vector<std::array<int, 2>>>
xyz2defects(const std::string &fname, InputInfo &info, ExtraInfo &extraInfo,
            const csaransh::Config &config);

std::pair<csaransh::DefectVecT, std::vector<std::array<int, 2>>>
displaced2defects(const std::string &fname, double lc);

std::pair<csaransh::DefectVecT, std::vector<std::array<int, 2>>>
atoms2defects(std::vector<std::tuple<Coords, double, Coords>> atoms,
              InputInfo &info, ExtraInfo &extraInfo, const csaransh::Config &config);

std::pair<csaransh::DefectVecT, std::vector<std::array<int, 2>>>
    displacedAtoms2defects(std::vector<std::array<csaransh::Coords, 2>> d,
                           double lc);
} // namespace csaransh

#endif // XYZ2DEFECTS_CSARANSH_HPP
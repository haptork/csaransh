/*!
 * @file
 * Function to find the histogram of angles, distances and adjacency as the
 * characteristic feature of a cluster shape
 * */
#ifndef CLUSTER2FEATURES_CSARANSH_HPP
#define CLUSTER2FEATURES_CSARANSH_HPP

#include <array>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <helper.hpp>

namespace csaransh {
// using defectsT = std::vector<std::tuple<Coords, bool, int, bool>>;

using distFeatT = std::array<double, 40>;
using angleFeatT = std::array<double, 36>;
// using angleFeatT = std::array<double, 180>;
using adjNn2FeatT = std::array<double, 20>;
using featT = std::tuple<distFeatT, angleFeatT, adjNn2FeatT>;
featT pairHists(const std::vector<std::array<double, 3>> &v,
                const std::vector<bool> &v2, double latConst);
}
#endif // CLUSTER2FEATURES_CSARANSH_HPP
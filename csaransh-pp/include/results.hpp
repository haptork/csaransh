/*!
 * @file
 * Finding different results like cluster size distribution, distance and
 * angles of defects from PKA origin.
 * */
#ifndef RESULTS_CSARANSH_HPP
#define RESULTS_CSARANSH_HPP

#include <array>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <helper.hpp>
#include <cluster2features.hpp>

namespace csaransh {

// coords, isInterstitial, ClusterId, isSurviving
using DefectT = std::tuple<std::array<double, 3>, bool, int, bool>;

using DefectVecT = std::vector<std::tuple<std::array<double, 3>, bool, int, bool>>;

struct ClusterSizeT {
  int surviving;
  int all;
};

using ClusterSizeMapT = std::unordered_map<int, ClusterSizeT>;

using ClusterIdMapT = std::unordered_map<int, std::vector<int>>;

using ClusterIVMapT = std::unordered_map<int, int>;

namespace DefectTWrap {
static inline auto coords(const DefectT &_d) { return std::get<0>(_d); }
static inline auto isInterstitial(const DefectT &_d) { return std::get<1>(_d); }
static inline auto clusterId(const DefectT &_d) { return std::get<2>(_d); }
static inline auto isSurviving(const DefectT &_d) { return std::get<3>(_d); }
static inline void coords(DefectT &_d, std::array<double, 3> c) { std::get<0>(_d) = c; }
static inline void isInterstitial(DefectT &_d, bool a) { std::get<1>(_d) = a; }
static inline void clusterId(DefectT &_d, int id) { std::get<2>(_d) = id; }
static inline void isSurviving(DefectT &_d, bool s) { std::get<3>(_d) = s; }
static inline auto isVacancy(const DefectT &_d) { return !std::get<1>(_d); }
}

DefectVecT groupDefects(const DefectVecT &defects, const double &latticeConst);

ClusterSizeMapT clusterSizes(const DefectVecT& defects);

void ignoreSmallClusters(DefectVecT& defects, ClusterSizeMapT&,
                         int minSurvived, int minAll);

ClusterIdMapT clusterMapping(const DefectVecT& defects);

ClusterIVMapT clusterIVType(const ClusterIdMapT&, ClusterSizeMapT&);

std::tuple<int, double, double> getNDefectsAndClusterFractions(const csaransh::DefectVecT& defects);

using ClusterFeatMapT = std::unordered_map<int, featT>;
ClusterFeatMapT clusterFeatures(const csaransh::DefectVecT& defects,
                        const csaransh::ClusterIdMapT& clusters,
                        csaransh::ClusterSizeMapT& clusterCounts,
                        double latticeConst);

std::tuple<int, int> getMaxClusterSizes(csaransh::ClusterSizeMapT& clusterCounts,
                  const csaransh::ClusterIdMapT& clusters);

std::array<std::vector<double>, 2> getDistanceDistribution(const DefectVecT &defects,
                                                const Info &info);
std::array<std::vector<double>, 2> getAngularDistribution(const DefectVecT &defects,
                                             const Info &info);

}
#endif //CSARANSH_RESULTS_HPP
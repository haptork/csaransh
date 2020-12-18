/*!
 * @file
 * Finding different features like cluster size distribution, distance and
 * angles of defects from PKA origin.
 * */
#include <cmath>

#include <UnionFind.hpp>
#include <helper.hpp>
#include <results.hpp>

#include <iostream>

// group defects into clusters
csaransh::DefectVecT csaransh::groupDefects(const csaransh::DefectVecT &defects,
                                            const double &latticeConst) {
  // std::cout << "Grouping defects " << defects.size() << '\n' << std::flush;
  using UF = csaransh::UnionFind<2, csaransh::DefectT>;
  auto nn = (std::sqrt(3) * latticeConst) / 2 + csaransh::invars::epsilon;
  auto nn2sqr = (latticeConst * latticeConst) + csaransh::invars::epsilon; // std::sqrt(3) * info.latticeConst + 0.01;
  auto nn4sqr = (nn * nn) * 4;
  auto pred = [nn2sqr, nn4sqr](const csaransh::DefectT &a,
                         const csaransh::DefectT &b) {
    using namespace DefectTWrap;
    if (isVacancy(a) && isVacancy(b) && isSurviving(a) && isSurviving(b)) { // both vacancies and surviving
      return calcDistSqr(coords(a), coords(b)) < nn4sqr;
    } 
    return calcDistSqr(coords(a), coords(b)) < nn2sqr;
  };
  UF uf;
  for (const auto &it : defects) {
    uf.uniteIf(it, pred);
  }
  // std::cout << "Finished grouping defects\n" << std::flush;
  return uf.getAll();
  // coords, isInterstitial, ClusterId, isSurviving
}

// cluster id and their sizes
csaransh::ClusterSizeMapT
csaransh::clusterSizes(const csaransh::DefectVecT &defects) {
  csaransh::ClusterSizeMapT clusterSize;
  using namespace csaransh::DefectTWrap;
  for (const auto &it : defects) {
    clusterSize[clusterId(it)].all++;
    if (!isSurviving(it)) continue;
    if (isInterstitial(it))
      clusterSize[clusterId(it)].surviving++;
    else
      clusterSize[clusterId(it)].surviving--;
  }
  return clusterSize;
}

csaransh::ClusterIVMapT
csaransh::clusterIVType(const csaransh::ClusterIdMapT &a,
                        csaransh::ClusterSizeMapT &b) {
  csaransh::ClusterIVMapT res;
  for (const auto &it : a)
    res[it.first] = b[it.first].surviving;
  return res;
}

// ignore dumbbells or similar defects group from cluster list
void csaransh::ignoreSmallClusters(csaransh::DefectVecT &defects,
                                   csaransh::ClusterSizeMapT &clusterSize,
                                   int minSurvived = 2, int minAll = 4) {
  using namespace csaransh::DefectTWrap;
  for (auto &it : defects) {
    if (abs(clusterSize[clusterId(it)].surviving) < minSurvived
        && clusterSize[clusterId(it)].all < minAll) {
      clusterId(it, 0); // setting clusterId of small ones to zero
    }
  }
}

// cluster ids mapped to defect ids that the clusters have
csaransh::ClusterIdMapT
csaransh::clusterMapping(const csaransh::DefectVecT &defects) {
  using namespace csaransh::DefectTWrap;
  csaransh::ClusterIdMapT clusterIds;
  int i = 0;
  for (const auto &it : defects) {
    if (clusterId(it) != 0) {
      clusterIds[clusterId(it)].push_back(
          i); // adding cluster ids and defect index
    }
    ++i;
  }
  return clusterIds;
}

// fraction of defects in cluster
std::tuple<int, double, double>
csaransh::getNDefectsAndClusterFractions(const csaransh::DefectVecT &defects) {
  using namespace csaransh::DefectTWrap;
  auto inClusterI = 0;
  auto inClusterV = 0;
  auto singlesI = 0;
  auto singlesV = 0;
  for (const auto &it : defects) {
    if (!isSurviving(it)) continue;
    if (clusterId(it) == 0) {
      if (isInterstitial(it))
        singlesI++;
      else
        singlesV++;
    } else {
      if (isInterstitial(it))
        inClusterI++;
      else
        inClusterV++;
    }
  }
  auto nDefects = inClusterI + singlesI;
  // auto nDefects = inClusterV + singlesV;
  double inClusterFractionI =
      (nDefects > 0) ? (double)(inClusterI)*100.0 / nDefects : 0;
  double inClusterFractionV =
      (nDefects > 0) ? (double)(inClusterV)*100.0 / nDefects : 0;
  return std::make_tuple(nDefects, inClusterFractionI, inClusterFractionV);
}

// cluster to cluster features mapping
csaransh::ClusterFeatMapT
csaransh::clusterFeatures(const csaransh::DefectVecT &defects,
                          const csaransh::ClusterIdMapT &clusters,
                          csaransh::ClusterSizeMapT &clusterCounts,
                          double latticeConst, int leastClusterSize) {
  using namespace csaransh::DefectTWrap;
  csaransh::ClusterFeatMapT clusterFeats;
  for (const auto &it : clusters) {
    if (clusterCounts[it.first].all < leastClusterSize) continue;
    std::vector<csaransh::Coords> clusterCoords;
    std::vector<bool> isI;
    for (const auto &jt : it.second) {
      auto x = coords(defects[jt]);
      clusterCoords.push_back(csaransh::Coords{{x[0], x[1], x[2]}});
      isI.push_back(isInterstitial(defects[jt]));
    }
    clusterFeats[it.first] =
        csaransh::pairHists(clusterCoords, isI, latticeConst);
  }
  return clusterFeats;
}

// maximum size of the interstitial and vacancy clusters
std::tuple<int, int>
csaransh::getMaxClusterSizes(csaransh::ClusterSizeMapT &clusterCounts,
                             const csaransh::ClusterIdMapT &clusters) {
  auto maxClusterSizeV = 0;
  auto maxClusterSizeI = 0;
  for (const auto &it : clusters) {
    const int sz = clusterCounts[it.first].surviving;
    if (sz > 0 && sz > maxClusterSizeI)
      maxClusterSizeI = sz;
    else if (sz < 0 && sz < maxClusterSizeV)
      maxClusterSizeV = sz;
  }
  return std::make_tuple(maxClusterSizeI, std::abs(maxClusterSizeV));
}
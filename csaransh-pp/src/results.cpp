/*!
 * @file
 * Finding different features like cluster size distribution, distance and
 * angles of defects from PKA origin.
 * */
#include <cmath>

#include <helper.hpp>
#include <results.hpp>
#include <UnionFind.hpp>

// group defects into clusters
csaransh::DefectVecT csaransh::groupDefects
    (const csaransh::DefectVecT &defects, const double &latticeConst) {
  using UF = csaransh::UnionFind<2, csaransh::DefectT>;
  auto nn = (std::sqrt(3) * latticeConst) / 2 + 1e-6;
  auto nn2 = latticeConst + 1e-6;//std::sqrt(3) * info.latticeConst + 0.01;
  auto nn4 = nn * 2;
  auto pred = [nn2, nn4](const csaransh::DefectT &a, const csaransh::DefectT &b) {
    using namespace DefectTWrap;
    if (isVacancy(a) && isVacancy(b)) {
      if (isSurviving(a) && isSurviving(b)) return calcDist(coords(a), coords(b)) < nn4; // both surviving
      return false;
    } else if (isInterstitial(a) && isInterstitial(b)) { // both interstitials
      if (!isSurviving(a) && isSurviving(b)) return calcDist(coords(a), coords(b)) < nn2; // exactly one surviving
      return calcDist(coords(a), coords(b)) < nn2; // both surviving
    } else if (isInterstitial(a) != isInterstitial(b)) {  // one interstitial and one vacancy
      //if (!get<3>(a) || !get<3>(b))
      if ((isVacancy(a) && !isSurviving(a)) || (isVacancy(b) && !isSurviving(b))) // if vacancy is not surviving
      return calcDist(coords(a), coords(b)) < nn2;
      return false;
    }
    return false;
  };
  UF uf;
  for (const auto &it : defects) {
    uf.uniteIf(it, pred);
  }
  return uf.getAll();
  // coords, isInterstitial, ClusterId, isSurviving
}

// cluster id and their sizes
csaransh::ClusterSizeMapT csaransh::clusterSizes(const csaransh::DefectVecT& defects) {
  csaransh::ClusterSizeMapT clusterSize;
  using namespace csaransh::DefectTWrap;
  for (const auto &it : defects) {
    clusterSize[clusterId(it)].all++;
    if (!isSurviving(it)) continue;
    if (isInterstitial(it)) clusterSize[clusterId(it)].surviving++;
    else clusterSize[clusterId(it)].surviving--;
  }
  return clusterSize;
}

// ignore dumbbells or similar defects group from cluster list
void csaransh::ignoreSmallClusters(csaransh::DefectVecT& defects, 
                          csaransh::ClusterSizeMapT& clusterSize,
                          int minSurvived = 2, int minAll = 4) {
  using namespace csaransh::DefectTWrap;
  for (auto &it : defects) {
    if (abs(clusterSize[clusterId(it)].surviving) < minSurvived && 
        clusterSize[clusterId(it)].all < minAll) {
          clusterId(it, 0); // setting clusterId of small ones to zero
    }
  }
}

// cluster ids mapped to defect ids that the clusters have
csaransh::ClusterIdMapT csaransh::clusterMapping(const csaransh::DefectVecT& defects) {
  using namespace csaransh::DefectTWrap;
  csaransh::ClusterIdMapT clusterIds;
  int i = 0;
  for (const auto &it : defects) {
    if (clusterId(it) != 0) {
      clusterIds[clusterId(it)].push_back(i); // adding cluster ids and defect index
    }
    ++i;
  }
  return clusterIds;
}

// fraction of defects in cluster
std::tuple<int, double, double> csaransh::getNDefectsAndClusterFractions(const csaransh::DefectVecT& defects) {
  using namespace csaransh::DefectTWrap;
  auto inClusterI = 0;
  auto inClusterV = 0;
  auto singlesI = 0;
  auto singlesV = 0;
  for (const auto &it : defects) {
    if (!isSurviving(it)) continue;
    if (clusterId(it) == 0) {
      if (isInterstitial(it)) singlesI++;
      else singlesV++;
    } else {
        if (isInterstitial(it)) inClusterI++;
        else inClusterV++;
    } 
  }
  auto nDefects = inClusterI + singlesI;
  double inClusterFractionI = (nDefects > 0) ? (double)(inClusterI) * 100.0 / nDefects : 0;
  double inClusterFractionV = (nDefects > 0) ? (double)(inClusterV) * 100.0 / nDefects : 0;
  return std::make_tuple(nDefects, inClusterFractionI, inClusterFractionV);
}

// cluster to cluster features mapping
csaransh::ClusterFeatMapT csaransh::clusterFeatures(const csaransh::DefectVecT& defects,
                        const csaransh::ClusterIdMapT& clusters,
                        csaransh::ClusterSizeMapT& clusterCounts,
                        double latticeConst) {
  using namespace csaransh::DefectTWrap;
  csaransh::ClusterFeatMapT clusterFeats;
  for (const auto &it : clusters) {
    std::vector<csaransh::Coords> clusterCoords;
    std::vector<bool> isI;
    for (const auto &jt : it.second) {
      auto x = coords(defects[jt]);
      clusterCoords.push_back(csaransh::Coords{{x[0], x[1], x[2]}});
      isI.push_back(isInterstitial(defects[jt]));
    }
    if (clusterCounts[it.first].all > 3) clusterFeats[it.first] = csaransh::pairHists(clusterCoords, isI, latticeConst);
  }
  return clusterFeats;
}

// maximum size of the interstitial and vacancy clusters
std::tuple<int, int> csaransh::getMaxClusterSizes(csaransh::ClusterSizeMapT& clusterCounts,
                  const csaransh::ClusterIdMapT& clusters) {
  auto maxClusterSizeV = 0;
  auto maxClusterSizeI = 0;
  for (const auto &it : clusters) {
    const int sz = clusterCounts[it.first].surviving;
    if (sz > 0 && sz > maxClusterSizeI) maxClusterSizeI = sz;
    else if (sz < 0 && sz < maxClusterSizeV) maxClusterSizeV = sz;
  }
  return std::make_tuple(maxClusterSizeI, std::abs(maxClusterSizeV));
}

// distance distribution of defects from PKA origin
std::array<std::vector<double>, 2> csaransh::getDistanceDistribution(const csaransh::DefectVecT &defects,
                          const csaransh::Info &info) {
  using namespace csaransh::DefectTWrap;
  std::vector<double> distsI;
  std::vector<double> distsV;
  std::array<double, 3> pka {{info.xrec, info.yrec, info.zrec}};
  for (const auto &it : defects) {
    if (!isSurviving(it)) continue;
    if (isInterstitial(it) == true) {
      distsI.emplace_back(csaransh::calcDist(pka, coords(it)));
    } else {
      distsV.emplace_back(csaransh::calcDist(pka, coords(it)));
    }
  }
  return std::array<std::vector<double>, 2>{{distsI, distsV}};
}

// angular distribution of defects from PKA origin
std::array<std::vector<double>, 2> csaransh::getAngularDistribution(const csaransh::DefectVecT &defects,
                         const csaransh::Info &info) {
  using namespace csaransh::DefectTWrap;
  std::vector<double> anglesI;
  std::vector<double> anglesV;
  std::array<double, 3> pka {{info.xrec, info.yrec, info.zrec}};
  for (const auto &it : defects) {
    if (!isSurviving(it)) continue;
    const auto &c = coords(it);
    auto angle = acos(((pka[0] - c[0]) * sin(info.rectheta) * cos(info.recphi) +
                       (pka[1] - c[1]) * sin(info.rectheta) * sin(info.recphi) +
                       (pka[2] - c[2]) * cos(info.rectheta)) / csaransh::calcDist(pka, c));
    angle  = (angle * 180) / 3.14159265;
    if (isInterstitial(it) == true) anglesI.emplace_back(angle);
    else anglesV.emplace_back(angle);
  }
  return std::array<std::vector<double>, 2>{{anglesI, anglesV}};
}
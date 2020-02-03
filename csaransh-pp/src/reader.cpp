#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>

#include <helper.hpp>
#include <infoReader.hpp>
#include <logger.hpp>
#include <printJson.hpp>
#include <reader.hpp>
#include <results.hpp>
#include <xyz2defects.hpp>

auto filterZeroClusters(csaransh::DefectVecT &defects,
                        csaransh::ClusterSizeMapT &clusterSize, bool isFilter) {
  using namespace csaransh::DefectTWrap;
  if (csaransh::Logger::inst().mode() & csaransh::LogMode::debug) {
    for (auto it : clusterSize) {
      if (it.second.surviving == 0) {
        csaransh::Logger::inst().log_debug(
            "Found cluster with zero size. (id, total defects): " +
            std::to_string(it.first) + ", " + std::to_string(it.second.all));
      }
    }
  }
  if (!isFilter) return;
  auto initSize = defects.size();
  defects.erase(
      std::remove_if(defects.begin(), defects.end(),
                     [&clusterSize](const auto &defect) {
                       return clusterSize[clusterId(defect)].surviving == 0;
                     }),
      defects.end());
}

csaransh::resultsT csaransh::process(csaransh::InputInfo &info,
                                     csaransh::ExtraInfo &extraInfo,
                                     const csaransh::Config &config) {
  auto res = csaransh::resultsT{};
  res.err = csaransh::ErrorStatus::noError;
  std::tie(res.defects, res.dumbellPairs) =
      (info.xyzFileType == csaransh::XyzFileType::lammpsDisplacedCompute)
          ? csaransh::displaced2defects(info.xyzFilePath, info.latticeConst)
          : csaransh::xyz2defects(info.xyzFilePath, info, extraInfo, config);
  if (res.defects.empty()) {
    res.err = csaransh::ErrorStatus::xyzFileDefectsProcessingError;
    return res;
  }
  res.defects =
      csaransh::groupDefects(std::move(res.defects), info.latticeConst);
  auto clusterSizeMap = csaransh::clusterSizes(res.defects);
  filterZeroClusters(res.defects, clusterSizeMap,
                     config.filterZeroSizeClusters);
  csaransh::ignoreSmallClusters(res.defects, clusterSizeMap, 2, 4);
  if (config.onlyDefects) {
    std::tie(res.nDefects, res.inClusterFractionI, res.inClusterFractionV) =
        csaransh::getNDefectsAndClusterFractions(res.defects);
    return res;
  }
  res.clusters = csaransh::clusterMapping(res.defects);
  res.clustersIV = csaransh::clusterIVType(res.clusters, clusterSizeMap);
  if (config.isFindClusterFeatures)
    res.feats = csaransh::clusterFeatures(res.defects, res.clusters,
                                          clusterSizeMap, info.latticeConst);
  int nDefects;
  std::tie(res.nDefects, res.inClusterFractionI, res.inClusterFractionV) =
      csaransh::getNDefectsAndClusterFractions(res.defects);
  std::tie(res.maxClusterSizeI, res.maxClusterSizeV) =
      csaransh::getMaxClusterSizes(clusterSizeMap, res.clusters);
  res.nClusters = res.clusters.size();
  if (!config.isFindDistribAroundPKA) return res;
  res.dists = csaransh::getDistanceDistribution(res.defects, extraInfo);
  res.angles = csaransh::getAngularDistribution(res.defects, extraInfo);
  return res;
}

csaransh::ErrorStatus csaransh::processFile(std::string xyzfile,
                                            std::ostream &outfile,
                                            const Config &config, std::string id) {
  std::string infile, tag;
  // std::cout << '\n' << xyzfile << '\n' << std::flush;
  std::tie(infile, tag) = csaransh::getInfileFromXyzfile(xyzfile);
  if (infile.empty()) return csaransh::ErrorStatus::inputFileMissing;
  csaransh::XyzFileType sc;
  bool status;
  std::tie(sc, status) = getSimulationCode(infile);
  if (!status) return csaransh::ErrorStatus::unknownSimulator;
  csaransh::InputInfo info;
  csaransh::ExtraInfo extraInfo;
  bool isInfo;
  std::tie(info, extraInfo, isInfo) =
      (sc == csaransh::XyzFileType::parcasWithStdHeader)
          ? csaransh::extractInfoParcas(infile, tag)
          : csaransh::extractInfoLammps(infile, tag);
  if (!isInfo) return csaransh::ErrorStatus::InputFileincomplete;
  extraInfo.id = id;
  info.xyzFileType = sc;
  info.xyzFilePath = xyzfile;
  auto res = csaransh::process(info, extraInfo, config);
  if (res.err == csaransh::ErrorStatus::noError) {
    csaransh::printJson(outfile, info, extraInfo, res);
  }
  return res.err;
}
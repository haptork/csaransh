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

std::pair<csaransh::ErrorStatus,int> csaransh::processFileTimeCmd(std::string xyzfileName,
                                            std::ostream &outfile,
                                            const csaransh::Config &config, int id, const csaransh::InputInfo &defaultInfo, const csaransh::ExtraInfo &defaultExtraInfo, bool isDefaultInfo) {
  std::string infileName, tag;
  std::tie(infileName, tag) = csaransh::getInfileFromXyzfile(xyzfileName);
  //if (infileName.empty()) return std::make_pair(csaransh::ErrorStatus::inputFileMissing, 0);
  csaransh::XyzFileType sc {csaransh::XyzFileType::generic};
  csaransh::InputInfo info;
  csaransh::ExtraInfo extraInfo;
  bool isInfo;
  if (infileName.empty()) {
    if (!isDefaultInfo) return std::make_pair(csaransh::ErrorStatus::inputFileMissing, 0);
    info = defaultInfo;
    extraInfo = defaultExtraInfo;
    isInfo = isDefaultInfo;
    sc = info.xyzFileType;
  } else {
    bool status;
    std::tie(sc, status) = csaransh::getSimulationCode(infileName);
    if (!status) return std::make_pair(csaransh::ErrorStatus::unknownSimulator, 0);
    std::tie(info, extraInfo, isInfo) =
      (sc == csaransh::XyzFileType::parcasWithStdHeader)
          ? csaransh::extractInfoParcas(infileName, tag)
          : csaransh::extractInfoLammps(infileName, tag);
    if (isDefaultInfo) Logger::inst().log_info("Found input file " + infileName);
  }
  if (!isInfo) return std::make_pair(csaransh::ErrorStatus::InputFileincomplete, 0);
  info.xyzFileType = sc;
  info.xyzFilePath = xyzfileName;
  csaransh::frameStatus fs = csaransh::frameStatus::prelude;
  std::ifstream xyzfile{info.xyzFilePath};
  if (xyzfile.bad() || !xyzfile.is_open()) return std::make_pair(csaransh::ErrorStatus::xyzFileReadError, 0);
  auto success = 0;
  auto frameCount = 0;
  while (true) {
    extraInfo.simulationTime = success + 1;
    extraInfo.id = std::to_string(id + success + 1);
    auto res = csaransh::processTimeFile(info, extraInfo, config, xyzfile, fs, outfile, success == 0);
    frameCount++;
    if (res.second != csaransh::ErrorStatus::noError) {
      if (config.allFrames) std::cerr << "\nError: " << errToStr(res.second) << " in frame " << frameCount << " of file " << xyzfileName << '\n' << std::flush;
      else std::cerr << "\nError: " << errToStr(res.second) << " of file " << xyzfileName << '\n' << std::flush;
      Logger::inst().log_info("Error processing" + std::to_string(frameCount) +" frame in file \"" + xyzfileName + "\"");
    } else {
      ++success;
      if (config.allFrames) {
        if (success >= 2) std::cout << "\r" << success << " steps processed successfully." << std::flush;
        Logger::inst().log_info("Finished processing" + std::to_string(success) +" frame in file \"" + xyzfileName + "\"");
      }
    }
    if (res.first == csaransh::xyzFileStatus::eof) break;
  }
  xyzfile.close();
  if (success > 0) return std::make_pair(csaransh::ErrorStatus::noError, success);
  return std::make_pair(csaransh::ErrorStatus::unknownError, 0);
}

std::pair<csaransh::xyzFileStatus, csaransh::ErrorStatus> 
                          csaransh::processTimeFile(csaransh::InputInfo &info,
                                     csaransh::ExtraInfo &extraInfo,
                                     const csaransh::Config &config, std::istream &infile, csaransh::frameStatus &fs, std::ostream &outfile, bool isFirst) {
  auto res = csaransh::resultsT{};
  //res.err = csaransh::ErrorStatus::noError;
  csaransh::xyzFileStatus fl;
  std::tie(fl, res.err, res.defects, res.coDefects) = 
      (info.xyzFileType == csaransh::XyzFileType::lammpsDisplacedCompute)
          ? csaransh::displaced2defectsTime(info, extraInfo, config, infile, fs)
          : csaransh::xyz2defectsTime(info, extraInfo, config, infile, fs);
  if (res.err != csaransh::ErrorStatus::noError) return std::make_pair(fl, res.err);
  res.defects = csaransh::groupDefects(std::move(res.defects), info.latticeConst);
  auto clusterSizeMap = csaransh::clusterSizes(res.defects);
  filterZeroClusters(res.defects, clusterSizeMap,
                     config.filterZeroSizeClusters);
  csaransh::ignoreSmallClusters(res.defects, clusterSizeMap);
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
  if (res.err == csaransh::ErrorStatus::noError) {
    if (!isFirst) outfile << "\n,";
    csaransh::printJson(outfile, info, extraInfo, res);
  }
  return std::make_pair(fl, res.err);
}
#include <algorithm>
#include <string>
#include <tuple>
#include <unordered_map>
#include <fstream>

#include <helper.hpp>
#include <reader.hpp>
#include <printJson.hpp>
#include <results.hpp>
#include <xyz2defects.hpp>

std::array<std::string, 2> separateDirAndFile(std::string path) {
  std::size_t dirPos = path.find_last_of("/");
  std::string dir{""};
  std::string file = path;
  if (dirPos != std::string::npos) {
    dir = path.substr(0, dirPos);
    file = path.substr(dirPos + 1);
  }
  return std::array<std::string, 2>{{dir, file}};
}

std::array<std::string, 2> separateFileAndExt(std::string path) {
  std::size_t extPos = path.find_last_of(".");
  std::string fname;
  if (extPos != std::string::npos)
    fname = path.substr(0, extPos);
  else
    return std::array<std::string, 2>{{path, fname}};
  std::string ext = path.substr(extPos + 1);
  return std::array<std::string, 2>{{fname, ext}};
}

std::pair<std::string, std::string>
csaransh::getInfileFromXyzfile(std::string xyzfile) {
  auto dirFile = separateDirAndFile(xyzfile);
  csaransh::replaceStr(dirFile[1], "fpos", "md");
  auto fnameExt = separateFileAndExt(dirFile[1]);
  auto addum = (dirFile[0].length() > 0) ? "/" : "";
  return std::make_pair(dirFile[0] + addum + fnameExt[0] + ".in", fnameExt[0]);
}

// extract information from input file
std::pair<csaransh::Info, bool> csaransh::extractInfoLammps(std::string fname) {
  std::ifstream infile(fname);
  csaransh::Info info;
  if (!infile.bad() && infile.is_open()) {
    std::string line;
    auto count = 0;
    std::array<bool, 3> xyzrec{{false, false, false}};
    csaransh::Coords velocity{{0.0, 0.0, 0.0}};
    while (std::getline(infile, line)) {
      auto eq = std::find_if(begin(line), end(line),
                             [](int ch) { return ch == '='; });
      if (eq != std::end(line)) {
        auto cmd = trim(std::string{std::begin(line), eq});
        ++eq;
        auto val = removeParcasComments(trim(std::string{eq, std::end(line)}));
        if (cmd == "substrate") {
          info.substrate = val;
          count++;
        } else if (cmd == "latticeConstant") {
          info.latticeConst = std::stod(val);
          count++;
        } else if (cmd == "ncell") {
          info.ncell = std::stod(val);
          count++;
        } else if (cmd == "recen") {
          info.energy = (std::stod(val)) / 1000.0;
          count++;
        } else if (cmd == "xrec") {
          info.xrec = std::stod(val);
          xyzrec[0] = true;
          count++;
        } else if (cmd == "yrec") {
          info.yrec = std::stod(val);
          xyzrec[1] = true;
          count++;
        } else if (cmd == "zrec") {
          info.zrec = std::stod(val);
          xyzrec[2] = true;
          count++;
        } else if (cmd == "vx") {
          velocity[0] = std::stod(val);
          count++;
        } else if (cmd == "vy") {
          velocity[1] = std::stod(val);
          count++;
        } else if (cmd == "vz") {
          velocity[2] = std::stod(val);
          count++;
        } else if (cmd == "offset") {
          info.origin = std::stod(val);
          count++;
        }
      }
    }
    auto infoExpected = 8;
    for (auto it : xyzrec)
      if (it) infoExpected++;
    info.structure =
        "bcc"; // TODO: extend for fcc, detect it without assumption
    infile.close();
    if (count == infoExpected) { // got all the info
      info.boxSize = info.latticeConst * info.ncell;
      info.infile = fname;
      info.rectheta = std::atan(velocity[1] / velocity[0]);
      info.recphi = std::atan(velocity[2] / velocity[0]);
      if (!xyzrec[0]) info.xrec = info.boxSize / 2.0;
      if (!xyzrec[1]) info.yrec = info.boxSize / 2.0;
      if (!xyzrec[2]) info.zrec = info.boxSize / 2.0;
      info.name = info.substrate + "_" + std::to_string((int)info.energy) +
                  "_" + std::to_string((int)info.rectheta) + "-" +
                  std::to_string((int)info.recphi);
      return std::make_pair(info, true);
    }
    return std::make_pair(info, false);
  }
  return std::make_pair(info, false);
}

// extract information from input file
std::pair<csaransh::Info, bool> csaransh::extractInfoParcas(std::string fname) {
  std::ifstream infile(fname);
  csaransh::Info info;
  if (!infile.bad() && infile.is_open()) {
    std::string line;
    auto count = 0;
    while (std::getline(infile, line)) {
      auto i = line.find('=');
      if (i == 9) {
        auto cmd = trim(line.substr(0, 9));
        if (cmd == "substrate") {
          info.substrate = removeParcasComments(line.substr(10));
          count++;
        } else if (cmd == "box(1)") {
          info.boxSize = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "ncell(1)") {
          info.ncell = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "recen") {
          info.energy =
              (std::stod(removeParcasComments(line.substr(10)))) / 1000.0;
          count++;
        } else if (cmd == "xrec") {
          info.xrec = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "yrec") {
          info.yrec = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "zrec") {
          info.zrec = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "rectheta") {
          info.rectheta = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "recphi") {
          info.recphi = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "offset(1)") {
          info.origin = std::stod(removeParcasComments(line.substr(10)));
          count++;
        }
      }
    }
    info.structure =
        "bcc"; // TODO: extend for fcc, detect it without assumption
    infile.close();
    if (count == 10) { // got all the info
      info.latticeConst = info.boxSize / info.ncell;
      info.infile = fname;
      info.name = info.substrate + "_" + std::to_string((int)info.energy) +
                  "_" + std::to_string((int)info.rectheta) + "-" +
                  std::to_string((int)info.recphi);
      return std::make_pair(info, true);
    }
    return std::make_pair(info, false);
  }
  return std::make_pair(info, false);
}

csaransh::readStatus processFileParcasLammps(std::string xyzfile,
                                             std::string infile,
                                             std::string tag,
                                             std::ofstream &outfile, int id,
                                             csaransh::SimulationCode sc) {
  csaransh::Info info;
  bool isInfo;
  std::tie(info, isInfo) = (sc == csaransh::SimulationCode::parcas)
                               ? csaransh::extractInfoParcas(infile)
                               : csaransh::extractInfoLammps(infile);
  info.infile = tag;
  if (!isInfo) return csaransh::readStatus::fail;
  info.simulationCode = sc;
  auto defects = csaransh::groupDefects(csaransh::xyz2defects(xyzfile, info),
                                        info.latticeConst);
  auto clusterSizeMap = csaransh::clusterSizes(defects);
  csaransh::ignoreSmallClusters(defects, clusterSizeMap, 2, 4);
  auto clusterIdMap = csaransh::clusterMapping(defects);
  auto clusterIVMap = csaransh::clusterIVType(clusterIdMap, clusterSizeMap);
  auto clusterFeats = csaransh::clusterFeatures(
      defects, clusterIdMap, clusterSizeMap, info.latticeConst);
  int nDefects;
  double inClusterFractionI, inClusterFractionV;
  std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
      csaransh::getNDefectsAndClusterFractions(defects);
  int maxClusterSizeI, maxClusterSizeV;
  std::tie(maxClusterSizeI, maxClusterSizeV) =
      csaransh::getMaxClusterSizes(clusterSizeMap, clusterIdMap);
  int nClusters = clusterIdMap.size();
  auto distances = csaransh::getDistanceDistribution(defects, info);
  auto angles = csaransh::getAngularDistribution(defects, info);
  csaransh::printJson(outfile, info, id, nDefects, nClusters, maxClusterSizeI,
                      maxClusterSizeV, inClusterFractionI, inClusterFractionV,
                      defects, distances, angles, clusterIdMap, clusterIVMap,
                      clusterFeats);
  return csaransh::readStatus::success;
}

csaransh::readStatus processFileDisplacedAtoms(std::string xyzfile,
                                               std::string infile,
                                               std::string tag,
                                               std::ofstream &outfile, int id,
                                               csaransh::SimulationCode sc) {
  csaransh::Info info;
  bool isInfo;
  std::tie(info, isInfo) = csaransh::extractInfoLammps(infile);
  info.infile = tag;
  if (!isInfo) return csaransh::readStatus::fail;
  info.simulationCode = sc;
  auto defects = csaransh::groupDefects(
      csaransh::displaced2defects(xyzfile, info.latticeConst),
      info.latticeConst);
  auto clusterSizeMap = csaransh::clusterSizes(defects);
  csaransh::ignoreSmallClusters(defects, clusterSizeMap, 2, 4);
  auto clusterIdMap = csaransh::clusterMapping(defects);
  auto clusterIVMap = csaransh::clusterIVType(clusterIdMap, clusterSizeMap);
  auto clusterFeats = csaransh::clusterFeatures(
      defects, clusterIdMap, clusterSizeMap, info.latticeConst);
  int nDefects;
  double inClusterFractionI, inClusterFractionV;
  std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
      csaransh::getNDefectsAndClusterFractions(defects);
  int maxClusterSizeI, maxClusterSizeV;
  std::tie(maxClusterSizeI, maxClusterSizeV) =
      csaransh::getMaxClusterSizes(clusterSizeMap, clusterIdMap);
  int nClusters = clusterIdMap.size();
  auto distances = csaransh::getDistanceDistribution(defects, info);
  auto angles = csaransh::getAngularDistribution(defects, info);
  csaransh::printJson(outfile, info, id, nDefects, nClusters, maxClusterSizeI,
                      maxClusterSizeV, inClusterFractionI, inClusterFractionV,
                      defects, distances, angles, clusterIdMap, clusterIVMap,
                      clusterFeats);
  return csaransh::readStatus::success;
}

std::pair<csaransh::SimulationCode, bool>
csaransh::getSimulationCode(std::string fname) {
  std::ifstream infile(fname);
  if (!infile.bad() && infile.is_open()) {
    std::string line;
    auto lineNo = 0;
    constexpr auto maxLinesToLook = 10;
    std::vector<std::string> keyWords{"PARCAS", "LAMMPS-XYZ", "LAMMPS-DISP"};
    std::vector<csaransh::SimulationCode> codes{
        csaransh::SimulationCode::parcas, csaransh::SimulationCode::lammps,
        csaransh::SimulationCode::lammpsDisplacedCompute};
    while (std::getline(infile, line) && lineNo++ < maxLinesToLook) {
      for (size_t i = 0; i < keyWords.size(); i++) {
        auto pos = line.find(keyWords[i]);
        if (pos != std::string::npos) return std::make_pair(codes[i], true);
      }
    }
  }
  return std::make_pair(csaransh::SimulationCode{}, false);
}

csaransh::readStatus csaransh::processFile(std::string xyzfile,
                                           std::ofstream &outfile, int id) {
  std::string infile, tag;
  std::tie(infile, tag) = csaransh::getInfileFromXyzfile(xyzfile);
  csaransh::SimulationCode sc;
  bool status;
  std::tie(sc, status) = getSimulationCode(infile);
  if (!status) return csaransh::readStatus::fail;
  if (sc == csaransh::SimulationCode::parcas ||
      sc == csaransh::SimulationCode::lammps) {
    return processFileParcasLammps(xyzfile, infile, tag, outfile, id, sc);
  } else if (sc == csaransh::SimulationCode::lammpsDisplacedCompute) {
    return processFileDisplacedAtoms(xyzfile, infile, tag, outfile, id, sc);
  }
  return csaransh::readStatus::fail;
}
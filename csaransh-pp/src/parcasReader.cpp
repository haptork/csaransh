#include <algorithm>
#include <string>
#include <tuple>
#include <unordered_map>
#include <fstream>

#include <helper.hpp>
#include <parcasReader.hpp>
#include <printJson.hpp>
#include <results.hpp>
#include <xyz2defects.hpp>

// trim a string from both ends
std::string csaransh::removeComments(std::string s) {
  ltrim(s);
  s.erase(std::find_if(s.begin(), s.end(), [](int ch) {
    return std::isspace(ch);
  }), s.end());
  return s;
}

// modified with a[i] + b[i] as denom
std::pair<csaransh::Info, bool> csaransh::extractInfo(std::string fname) {
  std::ifstream infile(fname);
  csaransh::Info info;
  if (!infile.bad() && infile.is_open()) {
    std::string line;
    size_t count = 0;
    while (std::getline(infile, line)) {
      auto i = line.find('=');
      if (i == 9) {
        auto cmd = trim(line.substr(0,9));
        if (cmd == "substrate") {
          info.substrate = removeComments(line.substr(10));
          count++;
        } else if (cmd == "box(1)") {
          info.boxSize = std::stod(removeComments(line.substr(10)));
          count++;
        } else if (cmd == "ncell(1)") {
          info.ncell = std::stod(removeComments(line.substr(10)));
          count++;
        } else if (cmd == "recen") {
          info.energy = (std::stod(removeComments(line.substr(10)))) / 1000.0;
          count++;
        } else if (cmd == "xrec") {
          info.xrec = std::stod(removeComments(line.substr(10)));
          count++;
        } else if (cmd == "yrec") {
          info.yrec = std::stod(removeComments(line.substr(10)));
          count++;
        } else if (cmd == "zrec") {
          info.zrec = std::stod(removeComments(line.substr(10)));
          count++;
        } else if (cmd == "rectheta") {
          info.rectheta = std::stod(removeComments(line.substr(10)));
          count++;
        } else if (cmd == "recphi") {
          info.recphi = std::stod(removeComments(line.substr(10)));
          count++;
        } else if (cmd == "offset(1)") {
          info.origin = std::stod(removeComments(line.substr(10)));
          count++;
        } 
      }
    }
    infile.close();
    if (count == 10) { // got all the info
      info.latticeConst = info.boxSize / info.ncell;
      info.infile = fname;
      info.name = info.substrate + "_" + std::to_string((int)info.energy) + "_" + 
                  std::to_string((int)info.rectheta) + "-" + std::to_string((int)info.recphi);
      return std::make_pair(info, true);
    }
    return std::make_pair(info, false);
  }
  return std::make_pair(info, false);
}

std::array<std::string, 2> csaransh::separateDirAndFile(std::string path) {
  std::size_t dirPos = path.find_last_of("/");
  std::string dir;
  if (dirPos != std::string::npos) dir = path.substr(0, dirPos);
  std::string file = path.substr(dirPos + 1);
  return std::array<std::string, 2>{{dir, file}};
}

std::pair<std::string, std::string> csaransh::getInfileFromXyzfile(std::string xyzfile) {
  auto dirFile = separateDirAndFile(xyzfile);
  csaransh::replaceStr(dirFile[1], "fpos", "md");
  auto tag = dirFile[1];
  csaransh::replaceStr(tag, ".xyz", "");
  csaransh::replaceStr(dirFile[1], "xyz", "in");
  return std::make_pair(dirFile[0] + "/" + dirFile[1], tag);
}

csaransh::readStatus csaransh::processParcasFile(std::string xyzfile, std::ofstream &outfile, int id) {
  std::string infile, tag;
  std::tie(infile, tag) = getInfileFromXyzfile(xyzfile);
  csaransh::Info info; bool isInfo;
  std::tie(info, isInfo) = extractInfo(infile);
  info.infile = tag;
  if (!isInfo) return readStatus::fail;
//csaransh::xyz2defects(xyzfile, info)
  auto defects = csaransh::groupDefects(csaransh::xyz2defects(xyzfile, info), info.latticeConst);
  auto clusterSizeMap = csaransh::clusterSizes(defects);
  csaransh::ignoreSmallClusters(defects, clusterSizeMap, 2, 4);
  auto clusterIdMap = csaransh::clusterMapping(defects);
  auto clusterFeats = csaransh::clusterFeatures(defects, clusterIdMap, clusterSizeMap, info.latticeConst);
  int nDefects;
  double inClusterFractionI, inClusterFractionV;
  std::tie(nDefects, inClusterFractionI, inClusterFractionV) = csaransh::getNDefectsAndClusterFractions(defects);
  int maxClusterSizeI, maxClusterSizeV;
  std::tie(maxClusterSizeI, maxClusterSizeV) = csaransh::getMaxClusterSizes(clusterSizeMap, clusterIdMap);
  int nClusters = clusterIdMap.size();
  auto distances = csaransh::getDistanceDistribution(defects, info);
  auto angles = csaransh::getAngularDistribution(defects, info);
  csaransh::printJson(outfile, tag, info, id, nDefects, nClusters, maxClusterSizeI, maxClusterSizeV,  inClusterFractionI, inClusterFractionV, defects, distances, angles, clusterIdMap, clusterFeats);
  return readStatus::success;
}
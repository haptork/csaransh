#include <algorithm>
#include <array>
#include <string>
#include <map>
#include <tuple>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <unordered_map>

#include <helper.hpp>

#include <dist.hpp>
#include <UnionFind.hpp>

using coordsT = std::array<double, 3>;
using defectsT = std::vector<std::tuple<coordsT, bool, int, bool>>;

using distFeatT = std::array<double, 40>;
using angleFeatT = std::array<double, 36>;
using adjNn2FeatT = std::array<double, 20>;
using adjNn4FeatT = std::array<double, 40>;
using featT = std::tuple<
  distFeatT, angleFeatT, adjNn2FeatT, adjNn4FeatT
>;
using featsT = std::unordered_map<int, featT>;

defectsT xyz2defects(std::string infile, Info info);

featT pairHists(const std::vector<std::array<double, 3>> &v, const std::vector<bool> &v2, double latConst);
// modified with a[i] + b[i] as denom

template <class T, size_t n>
auto printStdAr(const std::array<T, n> &ar, std::ofstream &outfile) {
    size_t count = 0;
    for (const auto &x : ar) {
      outfile << x ;
      if (count++ < (ar.size() - 1)) outfile << ", ";
    }
};

template <class T>
void printVector(const std::vector<T> &ar, std::ofstream &outfile) {
  size_t count = 0;
  for (const auto &x : ar) {
    outfile << x ;
    if (count++ < (ar.size() - 1)) outfile << ", ";
  }
};

static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
      return !std::isspace(ch);
  }));
}

static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
      return !std::isspace(ch);
  }).base(), s.end());
}

static inline auto trim(std::string s) {
  ltrim(s);
  rtrim(s);
  return s;
}

static inline auto removeComments(std::string s) {
  ltrim(s);
  s.erase(std::find_if(s.begin(), s.end(), [](int ch) {
    return std::isspace(ch);
  }), s.end());
  return s;
}

auto extractInfo(std::string fname) {
  std::ifstream infile(fname);
  Info info;
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

bool replace(std::string& str, const std::string& from, const std::string& to) {
  size_t start_pos = str.find(from);
  if(start_pos == std::string::npos)
      return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

auto separateDirAndFile(std::string path) {
  std::size_t dirPos = path.find_last_of("/");
  std::string dir;
  if (dirPos != std::string::npos) dir = path.substr(0, dirPos);
  std::string file = path.substr(dirPos + 1);
  return std::array<std::string, 2>{{dir, file}};
}

auto getInfileFromXyzfile(std::string xyzfile) {
  auto dirFile = separateDirAndFile(xyzfile);
  replace(dirFile[1], "fpos", "md");
  auto tag = dirFile[1];
  replace(tag, ".xyz", "");
  replace(dirFile[1], "xyz", "in");
  return std::make_tuple(dirFile[0] + "/" + dirFile[1], tag);
}

auto printClusterIds(const std::unordered_map<int, std::vector<int>> clusters, std::ofstream& outfile) {
  size_t i = 0;
  for (const auto &it : clusters) {
    outfile << '"' << it.first << "\":"; 
    outfile << "[";
    printVector(it.second, outfile);
    outfile << "]";
    if (i != clusters.size() - 1) outfile << ", ";
    outfile << "\n";
    ++i;
  }
}

auto printSingleFeat(const featT &feats, std::ofstream &outfile) {
  outfile << "\"dist\": ";
  outfile << "[";
  printStdAr(std::get<0>(feats), outfile);
  outfile << "],\n";
  outfile << "\"angle\": ";
  outfile << "[";
  printStdAr(std::get<1>(feats), outfile);
  outfile << "],\n";
  outfile << "\"adjNn2\": ";
  outfile << "[";
  printStdAr(std::get<2>(feats), outfile);
  /*
  outfile << "],\n";
  outfile << "\"adjNn4\": ";
  outfile << "[";
  printStdAr(std::get<3>(feats), outfile);
  */
  outfile << "]\n";
}
auto printFeats(const std::unordered_map<int, featT> &feats, std::ofstream &outfile) {
  size_t count = 0;
  for (const auto &it : feats) {
    outfile << '"' << it.first << "\":"; 
    outfile << "{";
    printSingleFeat(it.second, outfile);
    outfile << "}";
    if (count++ != feats.size() - 1) outfile << ", ";
    outfile << "\n";
  }
}

auto printJson(std::ofstream &outfile, const std::string &s, const Info &i, int id, const int &nDefects, const int &nClusters, 
               const int &maxClusterSizeI, const int &maxClusterSizeV, const double &inClusterFractionI, 
               const double &inClusterFractionV, const defectsT &defects, 
               const std::array<std::vector<double>, 2> &dists, const std::array<std::vector<double>, 2> &angles, 
               const std::unordered_map<int, std::vector<int>> clusters, const std::unordered_map<int, featT> &feats) {
  auto printDefects = [&outfile](const defectsT &d) {
    size_t count = 0;
    for (const auto &x : d) {
      outfile << "[" << std::get<0>(x)[0] << ", " << std::get<0>(x)[1] << ", " 
                << std::get<0>(x)[2] << ", " << std::get<1>(x) << ", " 
                << std::get<2>(x) << ", " << std::get<3>(x) << "]";
      if (count++ < (d.size() - 1)) outfile << ", ";
    }
  };
  outfile << "{";
  outfile << "\"infile\": \"" << i.infile << "\",\n"
            << "\"name\": \"" << i.name << "\",\n"
            << "\"id\": \"" << id << "\",\n"
            << "\"substrate\": \"" << i.substrate << "\",\n"
            << "\"boxSize\":" << i.boxSize << ",\n"
            << "\"energy\":" << i.energy << ",\n"
            << "\"ncell\":" << i.ncell << ",\n"
            << "\"rectheta\":" << i.rectheta << ",\n"
            << "\"recphi\":" << i.recphi << ",\n"
            << "\"xrec\":" << i.xrec << ",\n"
            << "\"yrec\":" << i.yrec << ",\n"
            << "\"zrec\":" << i.zrec << ",\n"
            << "\"latticeConst\":" << i.latticeConst << ",\n"
            << "\"origin\":" << i.origin << ",\n";
  outfile << "\"n_defects\":" << nDefects << ",\n"
            << "\"n_clusters\":" << nClusters << ",\n"
            << "\"max_cluster_size_I\":" << maxClusterSizeI << ",\n"
            << "\"max_cluster_size_V\":" << maxClusterSizeV << ",\n"
            << "\"max_cluster_size\":" << std::max(maxClusterSizeI, maxClusterSizeV) << ",\n"
            << "\"in_cluster_I\":" << inClusterFractionI << ",\n"
            << "\"in_cluster_V\":" << inClusterFractionV << ",\n"
            << "\"in_cluster\":" << (inClusterFractionI + inClusterFractionV) / 2.0 << ",\n";
  outfile << "\"coords\": [";
  printDefects(defects);
  outfile << "],\n";
  outfile << "\"clusters\": ";
  outfile << "{";
  printClusterIds(clusters, outfile);
  outfile << "}";
  outfile << ",\n";
  outfile << "\"features\": ";
  outfile << "{";
  printFeats(feats, outfile);
  outfile << "}";
  outfile << ",\n";
  outfile << "\"distancesI\": [";
  printVector(dists[0], outfile);
  outfile << "]";
  outfile << ",\n";
  outfile << "\"distancesV\": [";
  printVector(dists[1], outfile);
  outfile << "]";
  outfile << ",\n";
  outfile << "\"anglesI\": [";
  printVector(angles[0], outfile);
  outfile << "]";
  outfile << ",\n";
  outfile << "\"anglesV\": [";
  printVector(angles[1], outfile);
  outfile << "]\n";
  outfile << "}\n";
}

auto getClusterSizeDistribution(std::vector<std::tuple<std::array<double, 3>, bool, int, bool>> defects, const Info &info) {
  using ClustRowType = std::tuple<std::array<double, 3>, bool, int, bool>;
  using UF = UnionFind<2, ClustRowType>;
  auto nn = (std::sqrt(3) * info.latticeConst) / 2 + 1e-6;
  auto nn2 = info.latticeConst + 1e-6;//std::sqrt(3) * info.latticeConst + 0.01;
  auto nn4 = nn * 2;
  auto pred = [nn2, nn4](const ClustRowType &a, const ClustRowType &b) {
    if (!get<1>(a) && !get<1>(b)) { // both vacancies
      if (get<3>(a) && get<3>(b)) return calcDist(get<0>(a), get<0>(b)) < nn4; // both surviving
      return false;
    } else if (get<1>(a) && get<1>(b)) { // both interstitials
      if (!get<3>(a) && get<3>(b)) return calcDist(get<0>(a), get<0>(b)) < nn2; // exactly one surviving
      return calcDist(get<0>(a), get<0>(b)) < nn2; // both surviving
    } else if (get<1>(a) != get<1>(b)) {  // one interstitial and one vacancy
      //if (!get<3>(a) || !get<3>(b))
      if ((!get<1>(a) && !get<3>(a)) || (!get<1>(b) && !get<3>(b))) // if vacancy is not surviving
      return calcDist(get<0>(a), get<0>(b)) < nn2;
      return false;
    }
    return false;
  };
  UF uf;
  for (const auto &it : defects) {
    uf.uniteIf(it, pred);
  }
  auto res = uf.getAll();
  std::unordered_map<int, std::array<int, 2>> clusterCount; // only surviving, all
  for (const auto &it : res) {
    clusterCount[std::get<2>(it)][1]++;
    if (!std::get<3>(it)) continue;
    if (std::get<1>(it)) clusterCount[std::get<2>(it)][0]++;
    else clusterCount[std::get<2>(it)][0]--;
  }
  auto inClusterI = 0;
  auto inClusterV = 0;
  auto singlesI = 0;
  auto singlesV = 0;
  std::unordered_map<int, std::vector<int>> clusterIds;
  {
  int i = 0;
  for (auto &it : res) {
    if (abs(clusterCount[std::get<2>(it)][0]) < 2 && 
        clusterCount[std::get<2>(it)][1] < 4) { // dumbbells have 3
          std::get<2>(it) = 0;                  // setting small ones as zero
          if (std::get<3>(it)) {
            if (std::get<1>(it)) singlesI++;
            else singlesV++;
          } 
    } else {
      clusterIds[std::get<2>(it)].push_back(i); // adding cluster ids
      if (std::get<3>(it)) {
        if (std::get<1>(it)) inClusterI++;
        else inClusterV++;
      } 
    }
    ++i;
  }
  }
  std::unordered_map<int, featT> clusterFeats;
  for (const auto &it : clusterIds) {
    std::vector<coordsT> cluster;
    std::vector<bool> isI;
    for (const auto &jt : it.second) {
      auto x = std::get<0>(res[jt]);
      cluster.push_back(coordsT{{x[0], x[1], x[2]}});
      isI.push_back(std::get<1>(res[jt]));
    }
    if (clusterCount[it.first][1] > 3) clusterFeats[it.first] = pairHists(cluster, isI, info.latticeConst);
  }

  auto nClusters = 0;
  auto maxClusterSizeV = 0;
  auto maxClusterSizeI = 0;
  for (auto &it : clusterCount) {
    if (!(abs(it.second[0]) < 2 || 
        it.second[1] < 4)) {
          nClusters++;
    }
    if (it.second[0] > 0 && it.second[0] > maxClusterSizeI) maxClusterSizeI = it.second[0];
    else if (it.second[0] < 0 && it.second[0] < maxClusterSizeV) maxClusterSizeV = it.second[0];
  }
  auto nDefects = inClusterI + singlesI;
  double inClusterFractionI = (nDefects > 0) ? (double)(inClusterI) * 100.0 / nDefects : 0;
  double inClusterFractionV = (nDefects > 0) ? (double)(inClusterV) * 100.0 / nDefects : 0;
  return std::make_tuple(res, nDefects, nClusters, maxClusterSizeI, std::abs(maxClusterSizeV), 
                         inClusterFractionI, inClusterFractionV, clusterIds, clusterFeats);
}

auto getDistances(const std::vector<std::tuple<std::array<double, 3>, bool, int, bool>> &defects, const Info &info) {
  std::vector<double> distsI;
  std::vector<double> distsV;
  std::array<double, 3> pka {{info.xrec, info.yrec, info.zrec}};
  for (const auto &it : defects) {
    if (!std::get<3>(it)) continue;
    if (std::get<1>(it) == true) {
      distsI.emplace_back(calcDist(pka, std::get<0>(it)));
    } else {
      distsV.emplace_back(calcDist(pka, std::get<0>(it)));
    }
  }
  return std::array<std::vector<double>, 2>{{distsI, distsV}};
}

auto getAngles(const std::vector<std::tuple<std::array<double, 3>, bool, int, bool>> &defects, const Info &info) {
  std::vector<double> anglesI;
  std::vector<double> anglesV;
  std::array<double, 3> pka {{info.xrec, info.yrec, info.zrec}};
  for (const auto &it : defects) {
    if (!std::get<3>(it)) continue;
    const auto &c = std::get<0>(it);
    auto angle = acos(((pka[0] - c[0]) * sin(info.rectheta) * cos(info.recphi) +
                       (pka[1] - c[1]) * sin(info.rectheta) * sin(info.recphi) +
                       (pka[2] - c[2]) * cos(info.rectheta)) / calcDist(pka, c));
    angle  = (angle * 180) / 3.14159265;
    if (std::get<1>(it) == true) anglesI.emplace_back(angle);
    else anglesV.emplace_back(angle);
  }
  return std::array<std::vector<double>, 2>{{anglesI, anglesV}};
}

auto fn(std::string xyzfile, std::ofstream &outfile, int id) {
  std::string infile, tag;
  std::tie(infile, tag) = getInfileFromXyzfile(xyzfile);
  Info info; bool isInfo;
  std::tie(info, isInfo) = extractInfo(infile);
  info.infile = tag;
  std::unordered_map<int, featT> clusterFeats;
  if (!isInfo) return std::make_pair(1, clusterFeats);
  int nDefects, nClusters, maxClusterSizeI, maxClusterSizeV;
  double inClusterFractionI, inClusterFractionV;
  defectsT defects;
  std::unordered_map<int, std::vector<int>> clusterIds;
  std::tie(defects, nDefects, nClusters, maxClusterSizeI, maxClusterSizeV, 
           inClusterFractionI, inClusterFractionV, clusterIds, clusterFeats)
              = getClusterSizeDistribution(xyz2defects(xyzfile, info), info);
  auto distances = getDistances(defects, info);
  auto angles = getAngles(defects, info);
  printJson(outfile, tag, info, id, nDefects, nClusters, maxClusterSizeI, maxClusterSizeV,  inClusterFractionI, inClusterFractionV, defects, distances, angles, clusterIds, clusterFeats);
  return std::make_pair(0, clusterFeats);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "No input xyz file provided as cmd argument.\n";
    return 1;
  }
  auto src = std::string{};
  //const std::string path {"./public/js/cascades-data.js"};
  const std::string path {"./cascades-data.json"};
  std::ofstream outfile{path};
  if (!outfile.is_open()) {
    std::cerr << "The output path " + path + " is not accessible.\n";
    return 1;
  }
  outfile << "{\"source\": \"" << src << "\", \"data\": [\n";
  std::cout << "Total files to process: " << (argc - 1) << '\n' << std::flush;
  auto success = 0;
  std::vector<std::unordered_map<int, featT>> clusterFeats;
  for (int i = 1; i < argc; ++i) {
    std::cout << "\rCurrently processing file " << i << std::flush;
    int ret;
    std::unordered_map<int, featT> clusterFeat;
    std::tie(ret, clusterFeat) = fn(argv[i], outfile, i);
    if (ret) {
      std::cerr << "\nError in processing file " + std::string{argv[i]} + "\n";
    } else {
      ++success;
      if (i != argc - 1) outfile << ",";
      outfile << "\n";
      clusterFeats.emplace_back(std::move(clusterFeat));
    }
  }
  outfile << "]}" << "\n";
  outfile.close();
  std::cout << '\r' << success << " out of " << argc - 1 << " processed successfully.\n";
  return 0;
}

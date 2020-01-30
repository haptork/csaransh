/*!
 * @file
 * helper functions for general usage
 * */

#ifndef HELPER_CSARANSH_HPP
#define HELPER_CSARANSH_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <logger.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace csaransh {

enum class readStatus : bool { fail, success };

enum class XyzFileType : int {
  cascadesDbLikeCols,
  parcasWithStdHeader,
  lammpsWithStdHeader,
  lammpsDisplacedCompute
};

enum class ErrorStatus : int {
  inputFileMissing,
  InputFileincomplete,
  inputFileReadError,
  xyzFileDefectsProcessingError,
  xyzFileMissing,
  xyzFileReadError,
  unknownSimulator,
  unknownError,
  noError
};

std::string errToStr(ErrorStatus err);

struct InputInfo {
  int ncell{-1};
  double boxSize{-1.0};
  double latticeConst{-1.0};
  double originX;
  double originY;
  double originZ;
  int originType{0}; // 0->only given, 1-> only estimated, 2-> both
  double temperature{0.0};
  XyzFileType xyzFileType{};
  std::string xyzFilePath{""};
  std::string structure;
  // int latConstType{0}; // 0->only given, 1-> only optimized, 2-> both
};

struct ExtraInfo {
  double energy;
  double simulationTime;
  int id{1};
  // For distribution around PKA
  bool isPkaGiven{false};
  double xrec{0.0};
  double yrec{0.0};
  double zrec{0.0};
  double rectheta{0.0};
  double recphi{0.0};
  std::string substrate;
  std::string infile;
  std::string tags;
  std::string potentialUsed;
  std::string author;
};

struct Config {
  bool onlyDefects{false};
  bool isFindDistribAroundPKA{true};
  bool isFindClusterFeatures{true};
  bool filterZeroSizeClusters{false};
  bool isIgnoreBoundaryDefects{false};
  bool isAddThresholdInterstitials{true};
  bool safeRunChecks{true};
  double thresholdFactor{0.345};
  double extraDefectsSafetyFactor{50.0};
  int logMode{csaransh::LogMode::warning | csaransh::LogMode::error};
  std::string logFilePath{"log-csaransh-pp-cpp.txt"};
  std::string outputJSONFilePath{"cascades-data.json"};
};

using Coords = std::array<double, 3>;

static inline double calcDist(csaransh::Coords a, csaransh::Coords b) {
  double dist = 0.0;
  for (auto i : {0, 1, 2}) {
    dist += (a[i] - b[i]) * (a[i] - b[i]);
  }
  return std::sqrt(dist);
}

// write standard array as comma separated values
template <class T, size_t n>
auto writeStdAr(const std::array<T, n> &ar, std::ostream &outfile) {
  size_t count = 0;
  for (const auto &x : ar) {
    outfile << x;
    if (count++ < (ar.size() - 1)) outfile << ", ";
  }
};

// write standard vector as comma separated values
template <class T>
void writeVector(const std::vector<T> &ar, std::ostream &outfile) {
  size_t count = 0;
  for (const auto &x : ar) {
    outfile << x;
    if (count++ < (ar.size() - 1)) outfile << ", ";
  }
};

// left trim the given string
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](int ch) { return !std::isspace(ch); }));
}

// right trim the given string
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// trim a string from both ends and return as a new string
static inline auto trim(std::string s) {
  ltrim(s);
  rtrim(s);
  return s;
}

// replace a matched string from a string
static inline bool replaceStr(std::string &fromStr, const std::string &match,
                              const std::string &to) {
  size_t start_pos = fromStr.find(match);
  if (start_pos == std::string::npos) return false;
  fromStr.replace(start_pos, match.length(), to);
  return true;
}

// trim from left and remove parcas comments
// parcas comments are defined as: characters followed by a space in a value
static inline std::string removeParcasComments(std::string s) {
  ltrim(s);
  s.erase(
      std::find_if(s.begin(), s.end(), [](int ch) { return std::isspace(ch); }),
      s.end());
  return s;
}

template <typename T, size_t s> auto strAr(std::array<T, s> ar) {
  std::string res;
  for (auto it : ar) {
    res += std::to_string(it) + " ";
  }
  return res;
}

} // namespace csaransh
#endif

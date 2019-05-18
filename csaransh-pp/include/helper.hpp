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
#include <string>
#include <vector>

namespace csaransh {

enum class readStatus : bool { fail, success };

enum class SimulationCode : int { parcas, lammps, lammpsDisplacedCompute };

enum class ErrorStatus : int {
  inputFileMissing,
  InputFileincomplete,
  inputFileReadError,
  xyzFileMissing,
  xyzFileReadError,
  unknownError,
  noError
};

struct Info {
  int ncell;
  double boxSize;
  double energy;
  double rectheta;
  double recphi;
  double xrec;
  double yrec;
  double zrec;
  double latticeConst;
  double origin;
  std::string structure;
  std::string infile;
  std::string name;
  std::string substrate;
  SimulationCode simulationCode;
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
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
          }).base(), s.end());
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
  s.erase(std::find_if(s.begin(), s.end(), [](int ch) {
            return std::isspace(ch);
          }), s.end());
  return s;
}
}
#endif

/*!
 * @file
 * implements an O(log(N)) algorithm for detecting defects given a
 * single frame file having xyz coordinates of all the atoms
 * */
#ifndef XYZ2DEFECTS_CSARANSH_HPP
#define XYZ2DEFECTS_CSARANSH_HPP

#include <string>

#include <helper.hpp>
#include <results.hpp>

namespace csaransh {

enum class frameStatus : bool { prelude, inFrame };

enum class lineStatus : int { garbage, coords, frameBorder };

// Gives next expected lattice site given a lattice site
// in this way it enumerates all the lattice sites
// Currently only works for bcc
// TODO: extend for fcc
struct NextExpected {
private:
  double _min, _max, _minCur, _maxCur;
  Coords _cur;

public:
  NextExpected(double mn, double mx)
      : _min{mn}, _max{mx}, _minCur{mn}, _maxCur{mx - 0.5}, _cur{{mn, mn, mn}} {
  }
  NextExpected() = default;
  auto min(double x) {
    _min = x;
    _minCur = x;
  }
  auto max(double x) {
    _max = x;
    _maxCur = x - 0.5;
  }
  auto min() const { return _min; }
  const Coords &cur() const { return _cur; }
  bool allMax() const {
    constexpr auto epsilon = std::numeric_limits<double>::epsilon();
    for (const auto &x : _cur) {
      if (std::fabs(x - _max) > epsilon) return false;
    }
    return true;
  }
  const Coords &increment();
};

DefectVecT xyz2defects(const std::string &fname, const Info &info);
DefectVecT displaced2defects(const std::string &fname, double lc);

DefectVecT atoms2defects(std::vector<std::tuple<Coords, double, Coords>> atoms,
                         double lc);
DefectVecT
    displacedAtoms2defects(std::vector<std::array<csaransh::Coords, 2>> d,
                           double lc);

std::pair<lineStatus, Coords> getCoordLammps(const std::string &line);

std::pair<lineStatus, Coords> getCoordParcas(const std::string &line);

std::pair<lineStatus, std::array<Coords, 2>>
getCoordDisplaced(const std::string &line);
}

#endif // XYZ2DEFECTS_CSARANSH_HPP
#include <algorithm>
#include <array>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <cmath>

#include <AddOffset.hpp>
#include <helper.hpp>
#include <results.hpp>
#include <xyz2defects.hpp>

// Gives next expected lattice site given a lattice site
// in this way it enumerates all the lattice sites
// Currently only works for bcc
// TODO: extend for fcc
struct NextExpected {
private:
double _min, _max, minCur, maxCur;
const double epsilon = 1e-6;
public:
NextExpected(double mn, double mx) : _min{mn}, _max{mx}, minCur{mn}, maxCur{mx - 0.5} {}
NextExpected() = default;
auto min(double x) { _min = x; minCur = x; }
auto max(double x) { _max = x; maxCur = x - 0.5; }
auto min() { return _min; }
auto operator() (std::array<double, 3> c) {
  for (auto i : {2, 1, 0}) {
    if (c[i] < (maxCur - epsilon)) {
      if (i > 0) {
        c[i] += 1.0;
      } else { //if (i == 0)
        c[i] += 0.5;
        if (minCur == _min) {
          minCur = _min + 0.5;
          maxCur = _max;
        } else {
          minCur = _min;
          maxCur = _max - 0.5;
        }
      }
      for (auto j = i + 1; j <= 2; ++j) {
        c[j] = minCur;
      }
      return c;
    }
  }
  if (maxCur != _max) {
    maxCur = _max;
    minCur = _min + 0.5;
    c[0] = maxCur;
    c[1] = minCur;
    c[2] = minCur;
  }
  return c;
}

auto allMax(std::array<double, 3> c) {
  for (auto i : {0, 1, 2}) {
    if (std::fabs(c[i] - _max) > epsilon ) return false;
  }
  return true;
}
};

// tells if two coordinates are equal within epsilon = 1e-6 range
auto cmpApprox(const std::array<double, 3> &c1, const std::array<double, 3> &c2) {
  auto epsilon = 1e-6;
  for (auto i : {0, 1, 2}) {
    if (fabs(c1[i] - c2[i]) > epsilon) return (c1[i] > c2[i]) ? 1 : -1;
  }
  return 0;
}

// tells if a coordinate exists in vector of coordinates. The equality is
// measured by cmpApprox
auto existAnchorVac(const std::array<double, 3> &t, const std::vector<std::array<double, 3>> &v) {
  auto it = std::lower_bound(begin(v), end(v), t, [](const std::array<double, 3>& a, 
                                                     const std::array<double, 3>& b){
    if (cmpApprox(a, b) < 0) return true;
    return false;
  });
  if (it != std::end(v) && cmpApprox(*it, t) == 0) return true;
  return false;
}

// tells if a coordinate exists in vector of coordinates + bool. The equality is
// measured by cmpApprox
auto existAnchorInter(const std::array<double, 3> &t, 
                      const std::vector<std::tuple<std::array<double, 3>, bool>> &v) {
  auto temp = std::make_tuple(t, false);
  auto it = std::lower_bound(begin(v), end(v), temp, [](
    const std::tuple<std::array<double, 3>, bool>& a,
    const std::tuple<std::array<double, 3>, bool>& b) {
    if (cmpApprox(std::get<0>(a), std::get<0>(b)) < 0) return true;
    return false;
  });
  if (it != std::end(v) && cmpApprox(std::get<0>(*it), t) == 0) return true;
  return false;
}

// sets the surviving flag of an interstitial and vacancy if they are closer than a threshold
auto clean(std::vector<std::tuple<csaransh::Coords, csaransh::Coords, bool>> &inter, std::vector<std::tuple<csaransh::Coords, bool>> &vac, const csaransh::Info &info) {
  auto thresh = info.latticeConst;// * sqrt(3) / 2;
  for (size_t i = 0; i < vac.size(); ++i) {
    if (!std::get<1>(vac[i])) continue;
    auto min = thresh + 1e-6;
    size_t minj = 0;
    for (size_t j = 0; j < inter.size(); ++j) {
      if (!std::get<2>(inter[j])) continue;
      auto dist = csaransh::calcDist(std::get<0>(vac[i]), std::get<1>(inter[j]));
      if (dist < min) {
        min = dist;
        minj = j;
      }
    }
    if (min < thresh) {
      std::get<1>(vac[i]) = false;
      std::get<2>(inter[minj]) = false;
    }
  }
}

std::pair<csaransh::lineStatus, csaransh::Coords> getCoordLammps(std::string& line) {
  csaransh::Coords c;
  auto first = std::find_if(begin(line), end(line), [](int ch) {
    return !std::isspace(ch);
  });
  if (first == std::end(line)) return std::make_pair(csaransh::lineStatus::garbage, c); // possibly blank line
  auto second = std::find_if(first, end(line), [](int ch) {
    return std::isspace(ch);
  });
  std::string word{first, second};
  if (word == "ITEM:") {
    return std::make_pair(csaransh::lineStatus::frameBorder, c);
  }
  for (int i = 0; i < 3; ++i) {
    first = std::find_if(second, end(line), [](int ch) {
      return !std::isspace(ch);
    });
    second = std::find_if(first, end(line), [](int ch) {
      return std::isspace(ch);
    });
    if (first == second) {
      return std::make_pair(csaransh::lineStatus::garbage, c); // some other info
    }
    c[i] = std::stod(std::string{first, second});
  }
  std::cout << c[0] << ", " << c[1] << ", " << c[2] << '\n';
  return std::make_pair(csaransh::lineStatus::coords, c);
}

std::pair<csaransh::lineStatus, csaransh::Coords> getCoordParcas(std::string& line) {
  csaransh::Coords c;
  auto first = std::find_if(begin(line), end(line), [](int ch) {
    return !std::isspace(ch);
  });
  if (first == std::end(line)) return std::make_pair(csaransh::lineStatus::garbage, c); // possibly blank line
  if (std::isdigit(*first))  return std::make_pair(csaransh::lineStatus::garbage, c); // possibly first line with frame number
  auto second = std::find_if(first, end(line), [](int ch) {
    return std::isspace(ch);
  });
  std::string word{first, second};
  if (word == "Frame") {
    return std::make_pair(csaransh::lineStatus::frameBorder, c);
  }
  for (int i = 0; i < 3; ++i) {
    first = std::find_if(second, end(line), [](int ch) {
      return !std::isspace(ch);
    });
    second = std::find_if(first, end(line), [](int ch) {
      return std::isspace(ch);
    });
    c[i] = std::stod(std::string{first, second});
  }
  return std::make_pair(csaransh::lineStatus::coords, c);
}

std::vector<std::tuple<csaransh::Coords, double, csaransh::Coords>> getAtoms(const std::string &fname, const csaransh::Info &info) {
  using std::string; using std::vector; using std::tuple; using std::get;
  using csaransh::Coords;
  vector<tuple<Coords, double, Coords>> atoms; // for all the atoms along with nearest closest sites and offset
  std::ifstream infile{fname};
  if (infile.bad() || !infile.is_open()) return atoms;
  const auto latConst = info.latticeConst;
  auto o = info.origin;
  auto origin = std::array<double, 3> {{o, o, o}};
  auto obj = csaransh::AddOffset{latConst, info.structure, origin};
  atoms.reserve(info.ncell * info.ncell * info.ncell * 2);
  std::string line;
  // read file and apply object
  csaransh::frameStatus fs = csaransh::frameStatus::prelude;
  Coords c;
  csaransh::lineStatus ls;
  while (std::getline(infile, line)) {
    std::tie(ls, c) = getCoordParcas(line);
    if (ls == csaransh::lineStatus::coords && fs == csaransh::frameStatus::inFrame) {
      atoms.emplace_back(obj(c));
    } else if (ls == csaransh::lineStatus::frameBorder) {
      fs = csaransh::frameStatus::inFrame;
      if (fs != csaransh::frameStatus::prelude) {
        atoms.clear();  // Ignoring the frame before this one
      }
    }
  }
  infile.close();
  return atoms; 
}

// implements an O(log(N)) algorithm for detecting defects given a
// single frame file having xyz coordinates of all the atoms and input information
// Can be improved to O(N) as given here: https://arxiv.org/abs/1811.10923
csaransh::DefectVecT csaransh::xyz2defects(const std::string &fname, const csaransh::Info &info) {
  using std::vector; using std::tuple; using std::get; using csaransh::Coords;
  csaransh::DefectVecT defects;
  auto atoms = getAtoms(fname, info);
  if (atoms.empty()) return defects;
  std::sort(begin(atoms), end(atoms));
  auto lastRow = get<0>(atoms[atoms.size() - 1]);
  NextExpected nextExpected;
  nextExpected.max(lastRow[0]);
  nextExpected.min(get<0>(atoms[0])[0]);
  auto expected = Coords {{nextExpected.min(), nextExpected.min(), nextExpected.min()}};
  std::tuple<double, Coords, Coords> pre;
  bool isPre = false;
  const auto latConst = info.latticeConst;
  const auto thresh = 0.3 * latConst; // for rough interstitial threshold
  vector<tuple<Coords, Coords>> interThresh; // interstitials based on rough thresh
  vector<tuple<Coords, Coords, bool>> interstitials;
  vector<tuple<Coords, bool>> vacancies;
  for (const auto& row : atoms) {
    // threshold
    if (get<1>(row) > thresh) {
      interThresh.emplace_back(get<0>(row), get<2>(row));
    }
    // clean
    Coords ar = get<0>(row); double x = get<1>(row); Coords c = std::get<2>(row);
    using vecB = std::vector<std::tuple<Coords, double, Coords>>;
    if (cmpApprox(ar, expected) == 0) { // atom sits at correct lattice site
      expected = nextExpected(expected);
      pre = std::make_tuple(x, ar, c);
      isPre = true;
    } else if (cmpApprox(ar, expected) > 0) { // atom sits at lattice site more than currently expected
      vecB res;
      while (cmpApprox(ar, expected) > 0 && cmpApprox(expected, lastRow) < 0 && !nextExpected.allMax(expected)) {
        vacancies.emplace_back(expected, true);
        expected = nextExpected(expected);
      }
      expected = nextExpected(expected);
    } else { // atom sits at the same lattice site again
      vecB res;
      if (isPre && std::get<1>(pre) == ar) {
        auto isPreReal = std::get<0>(pre) > x;
        interstitials.emplace_back(std::get<1>(pre), std::get<2>(pre), isPreReal);
        interstitials.emplace_back(ar, c, !isPreReal); // both interstitials for structures like dumbbells
        vacancies.emplace_back(ar, false);  // dummy vacancy added
      } else {
        interstitials.emplace_back(ar, c, true);
      }
      isPre = false;
    }
  }
  atoms.clear();
  defects.reserve(2 * vacancies.size());
  auto vacCleanSave = vacancies;
  auto anchor2coord = [&latConst](Coords c) { for (auto& x : c) x *= latConst; return c; };
  auto count = 1;
  for (auto &it : vacancies) {
    for (auto& x : std::get<0>(it)) x *= latConst;
  }
  clean(interstitials, vacancies, info); // clean again
  for (const auto &it : interstitials) {
    defects.emplace_back(std::move(get<1>(it)), true, count++, get<2>(it));
  }
  for (const auto &it : vacancies) {
    defects.emplace_back(std::move(get<0>(it)), false, count++, get<1>(it));
  }
  for (const auto &it : interThresh) {
    if (!existAnchorInter(std::get<0>(it), vacCleanSave)) {
      defects.emplace_back(anchor2coord(std::move(get<0>(it))), false, count++, false);
      defects.emplace_back(std::move(get<1>(it)), true, count++, false);
    }
  }
  return defects;
}

/*
void testExpected(int argc, char* argv[]) {
    using std::stod;
    NextExpected nextExpected;
    nextExpected.min(stod(argv[4]));
    nextExpected.max(stod(argv[5]));
    auto c = std::array<double, 3> {{stod(argv[1]), stod(argv[2]), stod(argv[3])}};
    auto x = nextExpected(c);
    std::cout << x << '\n';
}

void testOffset(int argc, char *argv[]) {
  using std::stod;
  auto obj = AddOffset{2.87, "bcc", std::array<double, 3>{{0.25, 0.25, 0.25}}};
  //auto obj = AddOffset{1.00, "bcc", std::array<double, 3>{{0.0, 0.0, 0.0}}};
  auto c = std::array<double, 3> {{stod(argv[1]), stod(argv[2]), stod(argv[3])}};
  std::cout << obj(c) << '\n';
}
*/
